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
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "loop-invariant-code-motion"

namespace {

// Controlla se un'istruzione è loop-invariant
// Poiché stiamo lavorando dopo mem2reg, non abbiamo load/store esplicite
bool isLoopInvariant(const Instruction &I, const Loop *L) {
  // Se l'istruzione è una call che potrebbe avere side effects, non è invariante
  if (const CallInst *CI = dyn_cast<CallInst>(&I)) {
    if (CI->mayHaveSideEffects())
      return false;
  }
  
  // Controllo che tutti gli operandi siano loop-invariant
  for (const Use &U : I.operands()) {
    const Value *V = U.get();
    
    // Costanti e argomenti sono invarianti
    if (isa<Constant>(V) || isa<Argument>(V))
      continue;
    
    // Se l'operando è un'istruzione, deve essere definito fuori dal loop
    // o deve essere una phi node all'inizio del loop che prende valore solo dall'esterno
    if (const Instruction *OpInst = dyn_cast<Instruction>(V)) {
      if (L->contains(OpInst->getParent())) {
        // Caso speciale: se è una phi del header che prende valore solo dall'esterno
        // può essere considerata invariante
        if (const PHINode *PN = dyn_cast<PHINode>(OpInst)) {
          if (PN->getParent() == L->getHeader()) {
            bool AllIncomingFromOutside = true;
            for (unsigned i = 0; i < PN->getNumIncomingValues(); ++i) {
              if (L->contains(PN->getIncomingBlock(i))) {
                AllIncomingFromOutside = false;
                break;
              }
            }
            if (AllIncomingFromOutside)
              continue;
          }
        }
        return false;
      }
    }
  }
  
  return true;
}

// Controlla se è sicuro spostare un'istruzione fuori dal loop
// Semplificato per il contesto post-mem2reg
bool isSafeToHoist(const Instruction &I) {
  // Non possiamo spostare istruzioni che terminano il blocco
  if (I.isTerminator())
    return false;
  
  // Non possiamo spostare istruzioni phi
  if (isa<PHINode>(I))
    return false;
  
  // Nel contesto post-mem2reg, dobbiamo comunque verificare side effects per le call
  if (I.mayHaveSideEffects())
    return false;
  
  return true;
}

struct LoopCodeMotion : PassInfoMixin<LoopCodeMotion> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    
    bool Changed = false;
    
    // Debug: stampa info sulla funzione che stiamo analizzando
    dbgs() << "LICM: Analizzando funzione " << F.getName() << "\n";
    
    // Iteriamo su tutti i loop della funzione, partendo dai più interni
    SmallVector<Loop *, 8> WorkList;
    for (Loop *L : LI) {
      // Raccogliamo tutti i loop in un worklist in modo da processarli 
      // dal più interno al più esterno
      SmallVector<Loop *, 8> Loops;
      collectLoopsInPostOrder(L, Loops);
      WorkList.append(Loops.begin(), Loops.end());
    }
    
    // Processa i loop dal più interno al più esterno
    for (Loop *L : WorkList) {
      Changed |= hoistInstructions(L, LI, DT);
    }
    
    if (Changed) {
      // Se abbiamo modificato il programma, dobbiamo indicare quali analisi preserviamo
      PreservedAnalyses PA;
      PA.preserveSet<CFGAnalyses>();
      return PA;
    }
    return PreservedAnalyses::all();
  }
  
  // Utility per raccogliere i loop in post-order (dal più interno al più esterno)
  void collectLoopsInPostOrder(Loop *L, SmallVectorImpl<Loop *> &Loops) {
    for (Loop *SubLoop : L->getSubLoops()) {
      collectLoopsInPostOrder(SubLoop, Loops);
    }
    Loops.push_back(L);
  }
  
  // Applica l'ottimizzazione LICM a un singolo loop
  bool hoistInstructions(Loop *L, LoopInfo &LI, DominatorTree &DT) {
    bool Changed = false;
    BasicBlock *Preheader = L->getLoopPreheader();
    
    // Serve un preheader per spostare le istruzioni
    if (!Preheader)
      return false;
    
    // Collezioniamo le istruzioni loop-invariant
    SmallVector<Instruction *, 16> InvariantInsts;
    
    // Scorriamo i blocchi del loop
    for (BasicBlock *BB : L->blocks()) {
      // Saltiamo i blocchi dei loop annidati (li abbiamo già processati)
      if (LI.getLoopFor(BB) != L)
        continue;
      
      // Salta i nodi PHI nel loop header - li gestiamo separatamente
      bool IsHeader = (BB == L->getHeader());
      auto I = BB->begin();
      if (IsHeader) {
        while (I != BB->end() && isa<PHINode>(&*I))
          ++I;
      }
      
      // Cerca istruzioni invarianti
      for (; I != BB->end(); ++I) {
        Instruction &Inst = *I;
        
        // Verifica se l'istruzione è loop-invariant e può essere spostata in sicurezza
        if (isLoopInvariant(Inst, L) && isSafeToHoist(Inst)) {
          // Assicuriamoci che l'istruzione sia eseguita in modo sicuro
          bool IsSafe = true;
          
          // Se non siamo nel loop header, verifichiamo che il blocco sia eseguito in ogni iterazione
          if (!IsHeader) {
            // Verifica che il blocco contenente l'istruzione sia raggiungibile sempre in ogni iterazione
            // Questo richiede che il blocco sia dominato dal loop header e non ci siano salti condizionali
            // che possano saltare il blocco
            if (!DT.dominates(L->getHeader(), BB)) {
              IsSafe = false;
            }
          }
          
          if (IsSafe) {
            InvariantInsts.push_back(&Inst);
          }
        }
      }
    }
    
    // Ordiniamo le istruzioni invarianti per evitare dipendenze
    // Usiamo un algoritmo semplice: se A usa B, A deve venire dopo B
    for (size_t i = 0; i < InvariantInsts.size(); ++i) {
      for (size_t j = i + 1; j < InvariantInsts.size(); ++j) {
        // Controlla se InvariantInsts[j] dipende da InvariantInsts[i]
        for (const Use &U : InvariantInsts[j]->operands()) {
          if (U.get() == InvariantInsts[i]) {
            // Scambia le posizioni
            std::swap(InvariantInsts[i], InvariantInsts[j]);
            // Ricomincia da capo per questo livello
            i--;
            break;
          }
        }
      }
    }
    
    // Debug: stampa le istruzioni che verranno spostate
    dbgs() << "LICM: Loop con Header BB" << L->getHeader()->getName() << "\n";
    dbgs() << "Trovate " << InvariantInsts.size() << " istruzioni invarianti\n";
    for (Instruction *Inst : InvariantInsts) {
      dbgs() << "  Sposto: ";
      Inst->print(dbgs());
      dbgs() << "\n";
    }
    
    // Sposta le istruzioni invarianti fuori dal loop
    Instruction *InsertPoint = Preheader->getTerminator();
    for (Instruction *Inst : InvariantInsts) {
      Inst->moveBefore(InsertPoint);
      Changed = true;
    }
    
    return Changed;
  }
  
  static bool isRequired() { return true; }
};

} // namespace

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