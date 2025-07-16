#include <llvm/ADT/SetVector.h>
#include <llvm/Analysis/LoopInfo.h>
#include "llvm/IR/Dominators.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// Funzione per controllare se un'istruzione è loop invariant 
bool loopInvariant(SetVector<Instruction*> instInvarianti, Loop &L, Instruction &Inst) {
  bool ridefinitoDentroLoop;
  for (Value* op : Inst.operands()) { 
    if (isa<Constant>(op) || isa<Argument>(op)) continue;
    
    // L'operando è loop invariant se la sua definizione è esterna la loop,
    // oppure se dentro al loop, non è un PHINode ed è loop invariant 
    if (Instruction* I = dyn_cast<Instruction>(op)) { 
      ridefinitoDentroLoop = L.contains(I); 
      if (!ridefinitoDentroLoop || (!isa<PHINode>(I) && instInvarianti.contains(I))) 
        continue;  
    }
    return false;
  }
  return true;
}

// Funzione per controllare se l'istruzione ha dipendenze non instSpostate
bool haDipendenze(SetVector<Instruction*> &instSpostate, Loop &L, Instruction &I) {
  for (Value* op : I.operands()) {
    if (isa<Constant>(op) || isa<Argument>(op))
      continue;

    if (Instruction* opInst = dyn_cast<Instruction>(op)) {
      if (L.contains(opInst) && !instSpostate.contains(opInst))
        return true; 
    }
  }
  return false;
}

// Funzione per verificare code motion
bool spostabile(DominatorTree &DT, Loop &L, Instruction &I) {
  bool dominaUsciteLoop, dominaTuttiUsi;

  if (I.mayHaveSideEffects()) {
    outs() << "   Istruzione: " << I << "\n";
    errs() << "   ↳ NON movibile: istruzione ha effetti collaterali\n";
    return false;
  }

  SmallVector<BasicBlock*> loopExitBB; 
  L.getExitBlocks(loopExitBB);

  outs() << " - Istruzione: " << I;

  for (BasicBlock* block : loopExitBB) { 
    if (!DT.dominates(I.getParent(), block)) {
      dominaUsciteLoop = false;
      break;
    }
  }


  if (!dominaUsciteLoop) {
    for (Use &U : I.uses()) {
      if (Instruction* user = dyn_cast<Instruction>(U.getUser())) { 
        if (!L.contains(user)) {
          outs() << "\n";
          errs() << "   ↳ NON movibile: blocco non domina tutte le uscite e l'isruzione e usata fuori dal loop\n";
          return false;
        }
      }
    }
  }

  for (Use &U : I.uses()) {  
    if (PHINode* phi = dyn_cast<PHINode>(U.getUser())) {
      if (L.contains(phi)) {
        outs() << "\n";
        errs() << "   ↳ NON movibile: uso in PHI node (more reaching definitions)\n";
        return false;
      }
    }
    dominaTuttiUsi = DT.dominates(&I, U);  
    if (!dominaTuttiUsi) {
      outs() << "\n";
      errs() << "   ↳ NON movibile: istruzione non domina tutti i suoi usi\n";
      return false;
    }
  }

  outs() << "  ✔︎ Movibile\n";
  return true;
}


struct LoopInvariantCodeMotionPass : PassInfoMixin<LoopInvariantCodeMotionPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);

    for (Loop *L : LI) {   
      SetVector<Instruction*> instInvarianti;       
      SetVector<Instruction*> instMovibili;          
      SetVector<Instruction*> instSpostate;           

      SmallVector<BasicBlock*> exitBB;          
      L->getExitBlocks(exitBB);                 

      outs() << "loop: ";
      for (BasicBlock* BB : L->blocks()) {
        BB->printAsOperand(errs(), false); 
        outs() << " || ";

        for (Instruction &I : *BB){ 
          if (loopInvariant(instInvarianti, *L, I))
            instInvarianti.insert(&I); 
        }
      }

      outs() << "\n";
      for (Instruction* I : instInvarianti) {
        if (spostabile(DT, *L, *I)) 
          instMovibili.insert(I); 
      }

      outs() << "\n" << "Istruzioni loop invariant: \n";
      for (Instruction* I : instInvarianti) 
        outs() << "  --- " << *I << "\n";
      
      outs() << "Istruzioni instMovibili: \n";
      for (Instruction* I : instMovibili) 
        outs() << "  --- " << *I << "\n";

      bool changed;
      do {
        changed = false;
        for (auto I = instMovibili.rbegin(); I != instMovibili.rend(); ++I) {
          if (!instSpostate.contains(*I) && !haDipendenze(instSpostate, *L, **I)) {
            (*I)->moveBefore(L->getLoopPreheader()->getTerminator());
            instSpostate.insert(*I);
            changed = true;
          }
        }
      } while (changed);

      outs() << "Istruzioni spostate: \n";
      for (Instruction* I : instSpostate) {
        outs() << "  --- " << *I << "\n";
      }
    }

    return PreservedAnalyses::all();
  }
  
  static bool isRequired() { return true; }
};

llvm::PassPluginLibraryInfo getTestPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "LocalOpt", LLVM_VERSION_STRING, [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback([](StringRef Name, FunctionPassManager &FPM, 
                                      ArrayRef<PassBuilder::PipelineElement>) {
        if (Name == "li") {
          FPM.addPass(LoopInvariantCodeMotionPass());
          return true;
        }
        return false;
      });
    }
  };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return getTestPassPluginInfo();
}