#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Analysis/CFGPrinter.h"

using namespace llvm;

namespace {

struct LoopFusionPass : PassInfoMixin<LoopFusionPass> {
    bool canFuse(Loop *L1, Loop *L2) {
        // verifica che tutti i blocchi richiesti esistano
        BasicBlock *latch1 = L1->getLoopLatch();
        BasicBlock *latch2 = L2->getLoopLatch();
        BasicBlock *exit1 = L1->getUniqueExitBlock();
        BasicBlock *exit2 = L2->getUniqueExitBlock();
        BasicBlock *header1 = L1->getHeader();
        BasicBlock *header2 = L2->getHeader();
        BasicBlock *pre1 = L1->getLoopPreheader();
        BasicBlock *pre2 = L2->getLoopPreheader();

        return latch1 && latch2 && exit1 && exit2 && 
               header1 && header2 && pre1 && pre2;
    }

    bool haveSameIterations(Loop *L1, Loop *L2, ScalarEvolution &SE) {
        const SCEV *tripCount1 = SE.getBackedgeTakenCount(L1);
        const SCEV *tripCount2 = SE.getBackedgeTakenCount(L2);
        
        if (!tripCount1 || !tripCount2) {
            errs() << "impossibile calcolare il numero di iterazioni\n";
            return false;
        }
        
        errs() << "numero di iterazioni:\n";
        errs() << "  L1: " << *tripCount1 << "\n";
        errs() << "  L2: " << *tripCount2 << "\n";
        
        bool equal = tripCount1 == tripCount2;
        errs() << "  Uguali: " << (equal ? "si" : "no") << "\n";
        return equal;
    }

    bool areConnected(Loop *L1, Loop *L2) {
        return DirectlyConnected(L1, L2) || DirectlyConnected(L2, L1);
    }

    bool DirectlyConnected(Loop *L1, Loop *L2) {
        BasicBlock *Exit1 = L1->getUniqueExitBlock();
        BasicBlock *Pre2 = L2->getLoopPreheader();
        BasicBlock *Header1 = L1->getHeader();
        BasicBlock *Header2 = L2->getHeader();
        BasicBlock *Pre1 = L1->getLoopPreheader();
        
        if (!Exit1 || !Pre2) {
            errs() << "-> blocchi richiesti mancanti\n";
            return false;
        }

        // caso 1: connessione diretta
        if (Exit1 == Pre2) {
            errs() << "-> CASO 1: connessione diretta (exit = preheader)\n";
            return true;
        }
        for (BasicBlock *Succ : successors(Exit1))
            if (Succ == Pre2) {
                errs() << "-> CASO 1: connessione diretta (exit -> preheader)\n";
                return true;
            }

        errs() << "\n-> cerco pattern con guardia...\n";
        
        // caso 2: pattern con guardia
        BasicBlock *Guard = nullptr;
        
        for (BasicBlock *Pred : predecessors(Pre1)) {
            errs() << "  controllo predecessore del preheader: " << Pred->getName() << "\n";
            
            if (BranchInst *Branch = dyn_cast<BranchInst>(Pred->getTerminator())) {
                errs() << "    trovato branch: " << *Branch << "\n";
                
                if (!Branch->isConditional()) {
                    errs() << "    non è condizionale\n";
                    continue;
                }
                
                BasicBlock *Succ0 = Branch->getSuccessor(0);
                BasicBlock *Succ1 = Branch->getSuccessor(1);
                
                errs() << "    successore 0: " << Succ0->getName() << "\n";
                errs() << "    successore 1: " << Succ1->getName() << "\n";
                
                bool toL1_0 = (Succ0 == Pre1);
                bool toL1_1 = (Succ1 == Pre1);
                bool toL2_0 = (Succ0 == Pre2);
                bool toL2_1 = (Succ1 == Pre2);
                
                if ((toL1_0 && toL2_1) || (toL1_1 && toL2_0)) {
                    Guard = Pred;
                    errs() << "  trovata guardia!\n";
                    break;
                }
                
                errs() << "non porta ai loop corretti\n";
            } else {
                errs() << "non è un branch\n";
            }
        }

        if (Guard) {
            errs() << "-> CASO 2: pattern con guardia trovato\n";
            return true;
        }

        errs() << "-> nessuna connessione trovata\n";
        return false;
    }

    bool areCFEquiv(Loop *L0, Loop *L1, DominatorTree &DT, PostDominatorTree &PDT) {
        BasicBlock *Pre0 = L0->getLoopPreheader();
        BasicBlock *Pre1 = L1->getLoopPreheader();
        BasicBlock *Exit0 = L0->getUniqueExitBlock();
        BasicBlock *Exit1 = L1->getUniqueExitBlock();
        
        if (!Pre0 || !Pre1 || !Exit0 || !Exit1) {
            errs() << "Control Flow Equivalence Check: blocchi mancanti\n";
            return false;
        }
        
        bool L0DominatesL1 = DT.dominates(Pre0, Pre1);
        bool L1PostDominatesL0 = PDT.dominates(Exit1, Exit0);
        
        errs() << "Flow Equivalence check:\n";
        errs() << "  L0 domina L1: " << (L0DominatesL1 ? "si" : "no") << "\n";
        errs() << "  L1 post-domina L0: " << (L1PostDominatesL0 ? "si" : "no") << "\n";
        
        if (L0DominatesL1 && L1PostDominatesL0) {
            errs() << "  -> Loop sono control flow equivalent!\n";
            return true;
        }
        return false;
    }

    Value* findBaseIndexExpression(Value *V) {
        if (Instruction *I = dyn_cast<Instruction>(V)) {
            // Se è un'istruzione di cast (come sext), guarda l'operando
            if (CastInst *Cast = dyn_cast<CastInst>(I)) {
                return findBaseIndexExpression(Cast->getOperand(0));
            }
            // Se è un'operazione binaria, ritornala
            if (BinaryOperator *BinOp = dyn_cast<BinaryOperator>(I)) {
                return BinOp;
            }
        }
        return V;
    }

    bool areNegDistance(Loop *L1, Loop *L2) {
        errs() << "\n=== Analisi dipendenze negative ===\n";
        
        for (BasicBlock *BB1 : L1->getBlocks()) {
            for (Instruction &I1 : *BB1) {
                if (StoreInst *Store = dyn_cast<StoreInst>(&I1)) {
                    for (BasicBlock *BB2 : L2->getBlocks()) {
                        for (Instruction &I2 : *BB2) {
                            if (LoadInst *Load = dyn_cast<LoadInst>(&I2)) {
                                if (GetElementPtrInst *StoreGEP = dyn_cast<GetElementPtrInst>(Store->getPointerOperand())) {
                                    if (GetElementPtrInst *LoadGEP = dyn_cast<GetElementPtrInst>(Load->getPointerOperand())) {
                                        // verifica se operano sullo stesso array 
                                        if (StoreGEP->getPointerOperand() == LoadGEP->getPointerOperand()) {
                                            Value *LoadIdx = LoadGEP->getOperand(LoadGEP->getNumOperands()-1);
                                            Value *BaseExpr = findBaseIndexExpression(LoadIdx);
                                            
                                            // se l'indice è un'espressione add/sub
                                            if (BinaryOperator *BinOp = dyn_cast<BinaryOperator>(BaseExpr)) {
                                                if (BinOp->getOpcode() == Instruction::Add) {
                                                    Value *Op1 = BinOp->getOperand(0);
                                                    Value *Op2 = BinOp->getOperand(1);
                                                    
                                                    if (ConstantInt *Const = dyn_cast<ConstantInt>(Op2)) {
                                                        // se la costante è positiva, abbiamo una dipendenza negativa
                                                        if (Const->getSExtValue() > 0) {
                                                            errs() << "Trovata dipendenza negativa: accesso a[i+" 
                                                                   << Const->getSExtValue() << "]\n";
                                                            return true;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return false;
    }

    bool fuse(Loop *firstLoop, Loop *secondLoop, DominatorTree &DT, LoopInfo &LI) {
        // recupero tutti i blocchi necessari
        BasicBlock *Header1 = firstLoop->getHeader();
        BasicBlock *Header2 = secondLoop->getHeader();
        BasicBlock *Latch1 = firstLoop->getLoopLatch();
        BasicBlock *Latch2 = secondLoop->getLoopLatch();
        BasicBlock *Exit2 = secondLoop->getUniqueExitBlock();
        BasicBlock *Body1 = nullptr;
        BasicBlock *Body2 = nullptr;
        
        // cerco i blocchi body (primo blocco non-header e non-latch)
        for (BasicBlock *BB : firstLoop->getBlocks()) {
            if (BB != Header1 && BB != Latch1) {
                Body1 = BB;
                break;
            }
        }
        
        for (BasicBlock *BB : secondLoop->getBlocks()) {
            if (BB != Header2 && BB != Latch2) {
                Body2 = BB;
                break;
            }
        }
        
        if (!Header1 || !Header2 || !Body1 || !Body2 || !Latch1 || !Latch2 || !Exit2) {
            errs() << "Blocchi richiesti mancanti\n";
            return false;
        }

        errs() << "\n=== Inizio Fusione Loop ===\n";
        
        // Passo 1: sostituzione variabile induzione
        PHINode *IndVar1;
        PHINode *IndVar2;

        // recupero IndVar1 nel primo header
        for (BasicBlock::iterator I = Header1->begin(); isa<PHINode>(I); ++I) {
            PHINode *Phi = cast<PHINode>(I);
            IndVar1 = Phi;
            break;
        }

        // recupero IndVar2 nel secondo header
        for (BasicBlock::iterator I = Header2->begin(); isa<PHINode>(I); ++I) {
            PHINode *Phi = cast<PHINode>(I);
            IndVar2 = Phi;
            break;
        }

        if (!IndVar1 || !IndVar2) {
            errs() << "Variabili di induzione non trovate\n";
            return false;
        }

        // Debug della sostituzione delle variabili
        errs() << "\n=== debug sostituzione variabile induzione ===\n";
        errs() << "indVar1: " << *IndVar1 << "\n";
        errs() << "indVar2: " << *IndVar2 << "\n";

        // sostituisco tutti gli usi di IndVar2 con IndVar1 nei blocchi del secondo loop
        for (BasicBlock *BB : secondLoop->getBlocks()) {
            if (BB == Header2) continue;
            
            errs() << "\nprocesso blocco: " << BB->getName() << "\n";
            
            for (Instruction &I : *BB) {
                bool modified = false;
                for (unsigned i = 0; i < I.getNumOperands(); ++i) {
                    if (I.getOperand(i) == IndVar2) {
                        errs() << "  sostituisco in: " << I << "\n";
                        errs() << "    operando " << i << ": " << *IndVar2 << " -> " << *IndVar1 << "\n";
                        I.setOperand(i, IndVar1);
                        modified = true;
                    }
                }
                if (modified) {
                    errs() << "  risultato: " << I << "\n";
                }
            }
        }

        // Trasformazione CFG secondo schema
        errs() << "\ntrasformazione CFG:\n";

        // HeaderLoop1 -> L2Exit
        Instruction *Header1Term = Header1->getTerminator();
        for (unsigned i = 0; i < Header1Term->getNumSuccessors(); ++i) {
            if (Header1Term->getSuccessor(i) == Body1) {
                errs() << "  redirigo Header1 -> Exit2\n";
                Header1Term->setSuccessor(i, Exit2);
            }
        }

        // Body1 -> Body2
        Instruction *Body1Term = Body1->getTerminator();
        for (unsigned i = 0; i < Body1Term->getNumSuccessors(); ++i) {
            errs() << "  redirigo Body1 -> Body2\n";
            Body1Term->setSuccessor(i, Body2);
        }

        // Header2 -> Latch2
        Instruction *Header2Term = Header2->getTerminator();
        for (unsigned i = 0; i < Header2Term->getNumSuccessors(); ++i) {
            errs() << "  redirigo Header2 -> Latch2\n";
            Header2Term->setSuccessor(i, Latch2);
        }

        // Body2 -> Latch1
        Instruction *Body2Term = Body2->getTerminator();
        for (unsigned i = 0; i < Body2Term->getNumSuccessors(); ++i) {
            errs() << "  redirigo Body2 -> Latch1\n";
            Body2Term->setSuccessor(i, Latch1);
        }

        // rimuovo il secondo loop dalla LoopInfo
        LI.erase(secondLoop);

        // ricalcolo l'albero dei dominatori
        DT.recalculate(*Header1->getParent(), ArrayRef<DominatorTree::UpdateType>());

        errs() << "=== fusione loop completata ===\n";
        return true;
    }

    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
        errs() << "\n=== inizio analisi funzione: " << F.getName() << " ===\n";
        
        auto &LI = FAM.getResult<LoopAnalysis>(F);
        auto &DT = FAM.getResult<DominatorTreeAnalysis>(F);
        auto &PDT = FAM.getResult<PostDominatorTreeAnalysis>(F);
        auto &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

        SmallVector<Loop*, 8> Loops(LI.begin(), LI.end());
        bool modified = false;

        errs() << "trovati " << Loops.size() << " loops\n";

        for (size_t i = 0; i + 1 < Loops.size(); ++i) {
            Loop *L1 = Loops[i];
            Loop *L2 = Loops[i + 1];

            errs() << "\ncoppia loops -> " << i << " & " << (i+1) << ":\n";

            if (!canFuse(L1, L2)) {
                errs() << "impossibile fondere i loop: mancano blocchi latch o exit\n";
                continue;
            }

            if (!areConnected(L1, L2)) {
                errs() << "i loop non sono connessi nel CFG\n";
                continue;
            }

            errs() << "trovati loop adiacenti!\n";
            
            Loop *first = L1;
            Loop *second = L2;
            if (DirectlyConnected(L2, L1)) {
                errs() << "scambio ordine dei loop!\n";
                std::swap(first, second);
            }

            if (!areCFEquiv(first, second, DT, PDT)) {
                errs() << "i loop non sono control flow equivalent\n";
                continue;
            }

            if (!haveSameIterations(first, second, SE)) {
                errs() << "i loop hanno un numero diverso di iterazioni\n";
                continue;
            }

            if (areNegDistance(first, second)) {
                errs() << "trovata dipendenza negativa tra i loop\n";
                continue;
            }

            if (fuse(first, second, DT, LI)) {
                errs() << "fusione riuscita!\n";
                modified = true;
                Loops.erase(Loops.begin() + i + 1);
                --i;
            } else {
                errs() << "fusione fallita\n";
            }
        }

        errs() << "=== analisi funzione completata ===\n\n";
        return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
    }

    static bool isRequired() { return true; }
};

}

llvm::PassPluginLibraryInfo getLoopFusionPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "LoopFusionPass", LLVM_VERSION_STRING,
        [](PassBuilder &passBuilder) {
            passBuilder.registerPipelineParsingCallback(
                [](StringRef name, FunctionPassManager &functionPassManager,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (name == "lf") {
                        functionPassManager.addPass(LoopFusionPass());
                        return true;
                    }
                    return false;
                });
        }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return getLoopFusionPassPluginInfo();
}