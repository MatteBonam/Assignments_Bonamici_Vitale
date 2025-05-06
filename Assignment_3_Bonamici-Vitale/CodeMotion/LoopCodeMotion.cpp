#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "loop-invariant-code-motion"

namespace {
  
bool loopInvariant(const Instruction &I, const Loop *L) {
  if (const CallInst *CI = dyn_cast<CallInst>(&I)) {
    if (CI->mayHaveSideEffects()) return false;
  }

  for (const Use &U : I.operands()) {
    const Value *V = U.get();
    if (isa<Constant>(V) || isa<Argument>(V)) continue;
    if (const Instruction *OpInst = dyn_cast<Instruction>(V)) {
      if (L->contains(OpInst->getParent())) return false;
    }
  }
  return true;
}

bool spostabile(const Instruction &I) {
  if (I.isTerminator() || isa<PHINode>(I) || I.mayHaveSideEffects())
    return false;
  return true;
}

struct LoopCodeMotion : PassInfoMixin<LoopCodeMotion> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    bool Changed = false;

    for (Loop *L : LI) {
      BasicBlock *preheader = L->getLoopPreheader();
      if (!preheader) continue;

      SmallVector<Instruction *, 16> InvariantInsts;

      for (BasicBlock *BB : L->blocks()) {
        bool IsHeader = (BB == L->getHeader());
        auto I = BB->begin();
        if (IsHeader) {
          while (I != BB->end() && isa<PHINode>(&*I))
            ++I;
        }

        for (; I != BB->end(); ++I) {
          Instruction &Inst = *I;
          if (loopInvariant(Inst, L) && spostabile(Inst)) {
            InvariantInsts.push_back(&Inst);
          }
        }
      }

      Instruction *InsertPoint = preheader->getTerminator();
      for (Instruction *Inst : InvariantInsts) {
        Inst->moveBefore(InsertPoint);
        Changed = true;
      }
    }

    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

} 

llvm::PassPluginLibraryInfo getLoopCodeMotionPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, 
    "LoopCodeMotion", 
    LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM, 
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "li") {
            FPM.addPass(LoopCodeMotion());
            return true;
          }
          return false;
        });
    }
  };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getLoopCodeMotionPluginInfo();
}