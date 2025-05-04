#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Analysis/ValueTracking.h"

using namespace llvm;

namespace {

struct LoopCodeMotion : PassInfoMixin<LoopCodeMotion> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    
    bool Changed = false;
    
    for (Loop *L : LI) {
      Changed |= runOnLoop(L, DT);
    }
    
    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
  
  bool runOnLoop(Loop *L, DominatorTree &DT) {
    bool Changed = false;
    
    // Get preheader block
    BasicBlock *Preheader = L->getLoopPreheader();
    if (!Preheader) {
      return false; // Can't do LICM without a preheader
    }
    
    // Get exit blocks
    SmallVector<BasicBlock *, 4> ExitBlocks;
    L->getExitBlocks(ExitBlocks);
    
    // Process all blocks in the loop
    for (BasicBlock *BB : L->blocks()) {
      // Check if this block dominates all exit blocks
      bool DominatesAllExits = true;
      for (BasicBlock *ExitBB : ExitBlocks) {
        if (!DT.dominates(BB, ExitBB)) {
          DominatesAllExits = false;
          break;
        }
      }
      
      // Iterate through instructions in the block
      for (BasicBlock::iterator II = BB->begin(), IE = BB->end(); II != IE;) {
        Instruction *I = &*II++;
        
        // Skip non-movable instructions
        if (!isSafeToSpeculativelyExecute(I) || I->mayHaveSideEffects()) {
          continue;
        }
        
        // Check if instruction is loop-invariant
        if (isLoopInvariantInstruction(I, L)) {
          // Check if all operands are defined outside loop or by loop-invariant instructions
          bool AllOperandsInvariant = true;
          for (Value *Op : I->operands()) {
            if (Instruction *OpInst = dyn_cast<Instruction>(Op)) {
              if (L->contains(OpInst->getParent())) {
                AllOperandsInvariant = false;
                break;
              }
            }
          }
          
          if (AllOperandsInvariant) {
            // Additional safety checks
            if (DominatesAllExits || isSafeToMoveWithoutDominatingExits(I, L, DT)) {
              // Move the instruction to preheader
              I->moveBefore(Preheader->getTerminator());
              Changed = true;
            }
          }
        }
      }
    }
    
    return Changed;
  }
  
  bool isLoopInvariantInstruction(Instruction *I, Loop *L) {
    // An instruction is loop invariant if all its operands are:
    // 1. Constants, OR
    // 2. Defined outside the loop, OR
    // 3. Defined by loop-invariant instructions
    
    for (Value *Op : I->operands()) {
      if (isa<Constant>(Op)) {
        continue; // Constants are always invariant
      }
      
      if (Instruction *OpInst = dyn_cast<Instruction>(Op)) {
        if (L->contains(OpInst->getParent())) {
          // Operand is defined inside the loop - need to check if it's invariant
          if (!isLoopInvariantInstruction(OpInst, L)) {
            return false;
          }
        }
      }
    }
    
    return true;
  }
  
  bool isSafeToMoveWithoutDominatingExits(Instruction *I, Loop *L, DominatorTree &DT) {
    // Check if the value defined by this instruction is dead at all exits
    // This is a simplified check - a more complete implementation would
    // need to analyze all uses of the value
    
    // For this assignment, we'll just check if the instruction has no uses
    // outside the loop
    for (User *U : I->users()) {
      if (Instruction *UI = dyn_cast<Instruction>(U)) {
        if (!L->contains(UI->getParent())) {
          return false; // Used outside loop
        }
      }
    }
    
    return true;
  }

  static bool isRequired() { return true; }
};
} 

llvm::PassPluginLibraryInfo getLoopCodeMotionPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "LoopCodeMotion", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "li-opt") {
                    FPM.addPass(LoopCodeMotion());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getLoopCodeMotionPluginInfo();
}