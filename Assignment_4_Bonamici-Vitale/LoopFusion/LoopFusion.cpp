#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include <llvm/ADT/SetVector.h>
#include <llvm/Analysis/LoopInfo.h>
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/APInt.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

namespace {

static bool areAdjacent(Loop &L1, Loop &L2);
static bool haveSameIteration(Loop &L1, Loop &L2, ScalarEvolution &SE);
static bool isControlFlowEquivalent(Loop &L1, Loop &L2, 
                                  DominatorTree &DT, PostDominatorTree &PDT);
static bool haveNotNegativeMemoryDependencies(Loop &L1, Loop &L2, ScalarEvolution &SE, DependenceInfo &DI);
static bool haveNotNegativeScalarDependencies(Loop &L1, Loop &L2);
static bool haveNotNegativeDependencies(Loop &L1, Loop &L2, 
                                      ScalarEvolution &SE, DependenceInfo &DI);
static bool isLoopFusionValid(Loop *L1, Loop *L2, DominatorTree &DT, 
                            PostDominatorTree &PDT, ScalarEvolution &SE, 
                            DependenceInfo &DI);
static void printBlock(StringRef s, BasicBlock *BB);
static void mergeLoops(Loop *L1, Loop *L2, DominatorTree &DT, 
                     PostDominatorTree &PDT, ScalarEvolution &SE, 
                     DependenceInfo &DI, Function &F);

static int loop_counter;

// Controlla se i loop sono adiacenti (non ci sono statement in mezzo)
bool areAdjacent(Loop &L1, Loop &L2) {
  bool blocksAdjacent = L1.getExitBlock() == L2.getLoopPreheader();

  printBlock("L1 exit block", L1.getExitBlock());
  printBlock("L2 preheader block", L2.getLoopPreheader());

  return blocksAdjacent;                     
}

// Controlla se i loop hanno lo stesso numero di iterazioni
bool haveSameIteration(Loop &L1, Loop &L2, ScalarEvolution &SE) {
  const SCEV *S1 = SE.getBackedgeTakenCount(&L1);
  const SCEV *S2 = SE.getBackedgeTakenCount(&L2);
  
  if (isa<SCEVCouldNotCompute>(S1) || isa<SCEVCouldNotCompute>(S2)) {
    outs() << "-> Non posso calcolare il trip count per uno o entrambi i loop\n";
    return false;
  }
  
  outs() << "-> numero iter -> " << loop_counter << ": " 
         << *S1 << "\n";
  outs() << "-> numero iter ->  " << loop_counter+1 << ": " 
         << *S2 << "\n";
  
  return (S1 == S2);
}

// Controlla l'equivalenza del control flow verificando che il primo loop
// domini il secondo e che il secondo post-domini il primo
bool isControlFlowEquivalent(Loop &L1, Loop &L2, 
                           DominatorTree &DT, PostDominatorTree &PDT) {
  BasicBlock *L1_block = L1.getHeader();
  BasicBlock *L2_block = L2.getHeader();

  return (DT.dominates(L1_block, L2_block) && 
         PDT.dominates(L2_block, L1_block));
}

// Controlla le dipendenze di memoria negative
bool haveNotNegativeMemoryDependencies(Loop &L1, Loop &L2, 
                                     ScalarEvolution &SE, DependenceInfo &DI) {
  for(BasicBlock* BB: L1.blocks()) {
    for(Instruction &I : *BB) {
      auto *storeGEP = dyn_cast<GetElementPtrInst>(&I);
      if (!storeGEP) continue;
      
      auto *storeInst = dyn_cast<StoreInst>(storeGEP->getNextNode()); 
      if (!storeInst) continue;

      for (auto &U : storeGEP->getPointerOperand()->uses()) {
        Instruction* user = dyn_cast<Instruction>(U.getUser());
        if (!user || !L2.contains(user)) continue; 

        auto *storeOrLoadGEP = dyn_cast<GetElementPtrInst>(user);
        if (!storeOrLoadGEP) continue;

        outs() << "   Store instruction dopo GEP -> " << *storeGEP << "\n";
        outs() << "   Load instruction dopo GEP2 -> " << *storeOrLoadGEP << "\n";

        const SCEV *storeSCEV = SE.getSCEVAtScope(storeGEP, &L1);
        const SCEV *storeOrLoadSCEV = SE.getSCEVAtScope(storeOrLoadGEP, &L2);
        const SCEV *Diff = SE.getMinusSCEV(storeOrLoadSCEV, storeSCEV);
        const SCEV *temp = Diff; 
        const SCEVConstant *ConstDiff = dyn_cast<SCEVConstant>(temp);
        
        while(!ConstDiff && !temp->operands().empty()){
          temp = temp->operands()[0]; 
          ConstDiff = dyn_cast<SCEVConstant>(temp);
        }
        
        if (!ConstDiff) return false;
        
        int offset = ConstDiff->getValue()->getSExtValue();
        outs() << "   Offset: " << offset << "\n";

        const SCEVAddRecExpr *DiffRec = dyn_cast<SCEVAddRecExpr>(Diff);
        if (!DiffRec) return false;

        const SCEV *Step = DiffRec->getStepRecurrence(SE);
        const SCEVConstant *ConstStep = dyn_cast<SCEVConstant>(Step);
        if(!ConstStep) return false;

        int step = ConstStep->getValue()->getSExtValue();
        outs() << "   Step value: " << step << "\n";
      
        if ((step > 0 && offset > 0) || (step < 0 && offset < 0)) {
          outs() << "-> Dipendenza negativa trovata: offset " 
                << offset << " con step " << step << "\n";
          return false;
        }
      }  
    } 
  }
  return true;
}

// Controlla le dipendenze scalari negative
bool haveNotNegativeScalarDependencies(Loop &L1, Loop &L2) {
  for (BasicBlock *BB2 : L2.blocks()) {
    for (Instruction &I2 : *BB2) {
      for (Value *Op : I2.operands()) {
        if (Instruction *Def = dyn_cast<Instruction>(Op)) {
          if (L1.contains(Def) && !L1.isLoopInvariant(Def)) {
            outs() << "-> Dipendenza negativa: " << I2 
                  << " dipende da non-invariante: " << *Def << "\n";
            return false;
          }
        }
      }
    }
  }
  return true;
}

// Controllo combinato delle dipendenze
bool haveNotNegativeDependencies(Loop &L1, Loop &L2, 
                               ScalarEvolution &SE, DependenceInfo &DI) {
  return haveNotNegativeMemoryDependencies(L1, L2, SE, DI) && 
         haveNotNegativeScalarDependencies(L1, L2);
}

// Funzione per fondere due loop validi
void mergeLoops(Loop *L1, Loop *L2, DominatorTree &DT, 
               PostDominatorTree &PDT, ScalarEvolution &SE, 
               DependenceInfo &DI, Function &F) {
  BasicBlock *preHeaderL1 = L1->getLoopPreheader();
  BasicBlock *headerL1 = L1->getHeader();
  BasicBlock *latchL1 = L1->getLoopLatch();
  BasicBlock *firstBlockBodyL1 = headerL1->getTerminator()->getSuccessor(0);
  BasicBlock *lastBlockBodyL1 = latchL1->getSinglePredecessor();
  BasicBlock *exitingL1 = L1->getExitingBlock();
  BasicBlock *exitL1 = L1->getExitBlock();

  outs() << "*** L1 BLOCKS ***\n";
  printBlock("L1 PreHeader", preHeaderL1);
  printBlock("L1 Header", headerL1);
  printBlock("L1 First Block Body", firstBlockBodyL1);
  printBlock("L1 Last Block Body", lastBlockBodyL1);
  printBlock("L1 Latch", latchL1);
  printBlock("L1 Exiting Block", exitingL1);
  printBlock("L1 Exit Block", exitL1);

  BasicBlock *preHeaderL2 = L2->getLoopPreheader();
  BasicBlock *headerL2 = L2->getHeader();
  BasicBlock *latchL2 = L2->getLoopLatch();
  BasicBlock *firstBlockBodyL2 = headerL2->getTerminator()->getSuccessor(0);
  BasicBlock *lastBlockBodyL2 = latchL2->getSinglePredecessor();
  BasicBlock *exitingL2 = L2->getExitingBlock();
  BasicBlock *exitL2 = L2->getExitBlock();

  outs() << "*** L2 BLOCKS ***\n";
  printBlock("L2 PreHeader", preHeaderL2);
  printBlock("L2 Header", headerL2);
  printBlock("L2 First Block Body", firstBlockBodyL2);
  printBlock("L2 Last Block Body", lastBlockBodyL2);
  printBlock("L2 Latch", latchL2);
  printBlock("L2 Exiting Block", exitingL2);
  printBlock("L2 Exit Block", exitL2);

  // Sostituisce la variabile di induzione di L2 con quella di L1
  PHINode *inductionVariableL1 = L1->getCanonicalInductionVariable();
  PHINode *inductionVariableL2 = L2->getCanonicalInductionVariable();

  outs() << "IV L1: " << *inductionVariableL1 << "\n";
  outs() << "IV L2: " << *inductionVariableL2 << "\n";
  
  inductionVariableL2->replaceAllUsesWith(inductionVariableL1);
  inductionVariableL2->eraseFromParent();

  // Sposta le istruzioni dal preheader di L2 a L1
  std::vector<Instruction*> instPreHeaderL2toMove;
  for (Instruction &inst : *preHeaderL2) {
    if (&inst != preHeaderL2->getTerminator()) {
        instPreHeaderL2toMove.push_back(&inst);
    }
  }
  
  for (Instruction *inst : instPreHeaderL2toMove) {
    outs() << "Muovo inst dal preheader di L2: " << *inst << "\n";
    inst->moveBefore(preHeaderL1->getTerminator());
  }

  // Aggiorna i phi node
  preHeaderL2->replaceSuccessorsPhiUsesWith(preHeaderL1);
  latchL2->replaceSuccessorsPhiUsesWith(latchL1);

  // Sposta le istruzioni dall'header di L2 a L1
  std::vector<Instruction*> instHeaderL2ToMove;
  for (Instruction &inst : *headerL2) {
    if (&inst != headerL2->getTerminator())
      instHeaderL2ToMove.push_back(&inst);
  }

  for (Instruction *inst : instHeaderL2ToMove) {
    outs() << "Muovo inst da header di L2: " << *inst << "\n";
    if (isa<PHINode>(inst))
      inst->moveBefore(headerL1->getFirstNonPHI());
    else
      inst->moveBefore(headerL1->getTerminator());
  }

  // Aggiorna il control flow
  exitingL1->getTerminator()->setSuccessor(1, exitL2);
  lastBlockBodyL1->getTerminator()->setSuccessor(0, firstBlockBodyL2);
  lastBlockBodyL2->getTerminator()->setSuccessor(0, latchL1);

  // Pulisci i blocchi non raggiungibili
  EliminateUnreachableBlocks(F);  
  F.print(outs());
}

// Funzione di utilità per stampare i blocchi
void printBlock(StringRef s, BasicBlock *BB) {
  outs() << s << ": ";
  BB->printAsOperand(outs(), false);
  outs() << "\n";
}

// Verifica se la fusione dei loop è possibile
bool isLoopFusionValid(Loop *L1, Loop *L2, DominatorTree &DT, 
                      PostDominatorTree &PDT, ScalarEvolution &SE, 
                      DependenceInfo &DI) {

  if (!areAdjacent(*L1, *L2)) {
    outs() << "=> Loops non adiacenti\n";
    return false;
  }
  outs() << "=> Loop " << loop_counter << " e' adiacente con loop " 
         << loop_counter+1 << "\n";

  if(!haveSameIteration(*L1, *L2, SE)) {
    outs() << "=> Loops hanno iterazioni diverse\n";
    return false;
  }
  outs() << "=> Loop " << loop_counter << " e Loop " << loop_counter+1 
         << " hanno lo stesso numero di iterazioni\n";

  if(!isControlFlowEquivalent(*L1, *L2, DT, PDT)) {
    outs() << "=> Loops non control flow equivalenti\n";
    return false;
  }
  outs() << "=> Loop " << loop_counter << " e " << loop_counter+1 
         << " sono control flow equivalenti\n";

  if(!haveNotNegativeDependencies(*L1, *L2, SE, DI)) {
    outs() << "=> Loops hanno dipendenze negative\n";
    return false;
  }
  outs() << "=> Loop " << loop_counter << " e " << loop_counter+1 
         << " non hanno dipendenze negative \n";

  return true;
}

struct LoopFusionPass : public PassInfoMixin<LoopFusionPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
    ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
    DependenceInfo &DI = AM.getResult<DependenceAnalysis>(F);
    
    if (LI.empty()) {
      outs() << "Function " << F.getName() << ": nessun loop trovato.\n";
      return PreservedAnalyses::all();
    }
    
    outs() << "\n*** LoopFusionPass ***\n";
    outs() << "=== Function: " << F.getName() << " ===\n\n";

    loop_counter = 1;  
    auto L1 = LI.rbegin();
    auto L2 = std::next(L1);
    
    while (L2 != LI.rend()){
      outs() << "* Controllo Loop " << loop_counter << " e Loop " 
             << loop_counter+1 << " *\n";
      
      if(isLoopFusionValid(*L1, *L2, DT, PDT, SE, DI)){
        outs() << "\nLoop " << loop_counter << " e Loop " 
               << loop_counter+1 << " possono essere fusi\n\n";

        mergeLoops(*L1, *L2, DT, PDT, SE, DI, F);
        
        DT.recalculate(F);  
        PDT.recalculate(F);
        SE.forgetLoop(*L1);
        
        LI.releaseMemory(); 
        LI.analyze(DT);

        L1 = LI.rbegin();  
        for(int i = 1; i < loop_counter; i++) {
          L1++;
        }
        L2 = std::next(L1);

        outs() << "\nLoop fusi - L2 rimosso e L1 aggiornato\n";
      } else {
        loop_counter++;
        L1++;
        L2 = std::next(L1);
      }
      
      outs() << "\n";
    }
    
    return PreservedAnalyses::all();
  }
};

} // namespace

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION,
    "LoopFusion",
    LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
          [](StringRef Name, FunctionPassManager &FPM,
             ArrayRef<PassBuilder::PipelineElement>) {
            if (Name == "lf") {
              FPM.addPass(LoopFusionPass());
              return true;
            }
            return false;
          });
    }
  };
}