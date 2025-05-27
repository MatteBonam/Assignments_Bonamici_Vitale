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
        // Check all required blocks exist
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
            errs() << "Trip count not computed\n";
            return false;
        }
        
        errs() << "Trip counts:\n";
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
        BasicBlock *Header1 = firstLoop->getHeader();
        BasicBlock *Body1 = firstLoop->getLoopLatch();
        BasicBlock *Exit1 = firstLoop->getUniqueExitBlock();
        BasicBlock *Pre1 = firstLoop->getLoopPreheader();
        BasicBlock *Header2 = secondLoop->getHeader();
        BasicBlock *Body2 = secondLoop->getLoopLatch();
        BasicBlock *Exit2 = secondLoop->getUniqueExitBlock();
        
        if (!Header1 || !Body1 || !Exit1 || !Pre1 || !Header2 || !Body2 || !Exit2) {
            errs() << "Blocchi necessari mancanti per la fusione\n";
            return false;
        }

        // variabili di induzione
        PHINode *Ind1 = nullptr;
        PHINode *Ind2 = nullptr;
        Value *StepVal = nullptr;
        
        // PHI nodes header
        for (PHINode &PHI : Header1->phis()) {
            // se è una PHI node che incrementa di 1, è probabilmente la nostra variabile di induzione
            if (auto *IncInst = dyn_cast<BinaryOperator>(PHI.getIncomingValueForBlock(Body1))) {
                if (IncInst->getOpcode() == Instruction::Add) {
                    if (auto *Const = dyn_cast<ConstantInt>(IncInst->getOperand(1))) {
                        if (Const->getValue() == 1) {
                            Ind1 = &PHI;
                            StepVal = IncInst;
                            break;
                        }
                    }
                }
            }
        }
        
        for (PHINode &PHI : Header2->phis()) {
            if (auto *IncInst = dyn_cast<BinaryOperator>(PHI.getIncomingValueForBlock(Body2))) {
                if (IncInst->getOpcode() == Instruction::Add) {
                    if (auto *Const = dyn_cast<ConstantInt>(IncInst->getOperand(1))) {
                        if (Const->getValue() == 1) {
                            Ind2 = &PHI;
                            break;
                        }
                    }
                }
            }
        }
        
        if (!Ind1 || !Ind2) {
            errs() << "Variabili di induzione non trovate\n";
            return false;
        }

        errs() << "Trovate variabili di induzione:\n";
        errs() << "  Loop1: " << *Ind1 << "\n";
        errs() << "  Loop2: " << *Ind2 << "\n";

        // sostituzione usi della variabile di induzione del secondo loop nel suo body
        for (Instruction &I : *Body2) {
            for (unsigned i = 0; i < I.getNumOperands(); ++i) {
                if (I.getOperand(i) == Ind2) {
                    I.setOperand(i, Ind1);
                }
            }
        }

        // 3. Modifica il CFG:
        // - Il body del primo loop deve saltare al body del secondo
        // - Il body del secondo loop deve saltare al header del primo
        // - Il header del primo loop deve saltare all'exit del secondo quando finisce
        
        // Modifica il branch del header1 per saltare a Exit2
        BranchInst *HeaderTerm1 = dyn_cast<BranchInst>(Header1->getTerminator());
        if (!HeaderTerm1) {
            errs() << "Terminatore dell'header del primo loop non trovato\n";
            return false;
        }
        
        Value *Cond = nullptr;
        if (HeaderTerm1->isConditional()) {
            Cond = HeaderTerm1->getCondition();
        }
        HeaderTerm1->eraseFromParent();
        
        if (Cond) {
            BranchInst::Create(Body1, Exit2, Cond, Header1);
        } else {
            BranchInst::Create(Body1, Header1);
        }

        // Modifica il branch del body1 per saltare al body2
        BranchInst *Term1 = dyn_cast<BranchInst>(Body1->getTerminator());
        if (!Term1) {
            errs() << "Terminatore del primo body non trovato\n";
            return false;
        }
        Term1->eraseFromParent();
        BranchInst::Create(Body2, Body1);

        // Modifica il branch del body2 per saltare al header1
        BranchInst *Term2 = dyn_cast<BranchInst>(Body2->getTerminator());
        if (!Term2) {
            errs() << "Terminatore del secondo body non trovato\n";
            return false;
        }
        Term2->eraseFromParent();
        BranchInst::Create(Header1, Body2);

        // 4. Aggiorna la PHI node del primo loop per riflettere il nuovo CFG
        Value *InitVal = Ind1->getIncomingValueForBlock(Pre1);
        Ind1->removeIncomingValue(Body1);
        Ind1->addIncoming(StepVal, Body2);

        // 5. Rimuovi la PHI node del secondo loop
        Ind2->replaceAllUsesWith(UndefValue::get(Ind2->getType()));
        Ind2->eraseFromParent();

        // 6. Aggiorna il DominatorTree
        DT.recalculate(*firstLoop->getHeader()->getParent());

        errs() << "Fusione completata con nuovo pattern:\n";
        errs() << "  - Header1 -> Exit2\n";
        errs() << "  - Body1 -> Body2\n";
        errs() << "  - Body2 -> Header1\n";
        
        return true;
    }

    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
        errs() << "\n=== Starting function analysis: " << F.getName() << " ===\n";
        
        auto &LI = FAM.getResult<LoopAnalysis>(F);
        auto &DT = FAM.getResult<DominatorTreeAnalysis>(F);
        auto &PDT = FAM.getResult<PostDominatorTreeAnalysis>(F);
        auto &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

        SmallVector<Loop*, 8> topLevelLoops(LI.begin(), LI.end());
        bool modified = false;

        errs() << "Found " << topLevelLoops.size() << " top-level loops\n";

        for (size_t i = 0; i + 1 < topLevelLoops.size(); ++i) {
            Loop *L1 = topLevelLoops[i];
            Loop *L2 = topLevelLoops[i + 1];

            errs() << "\nCoppia loops -> " << i << " & " << (i+1) << ":\n";

            if (!canFuse(L1, L2)) {
                errs() << "Loops cannot be fused: missing latch or exit block\n";
                continue;
            }

            if (!areConnected(L1, L2)) {
                errs() << "Loops are not connected in CFG\n";
                continue;
            }

            errs() << "Trovati loop adiacenti!\n";
            
            Loop *first = L1;
            Loop *second = L2;
            if (DirectlyConnected(L2, L1)) {
                errs() << "Swappo ordine loops!\n";
                std::swap(first, second);
            }

            if (!areCFEquiv(first, second, DT, PDT)) {
                errs() << "Loops are not control flow equivalent\n";
                continue;
            }

            if (!haveSameIterations(first, second, SE)) {
                errs() << "Loops have different number of iterations\n";
                continue;
            }

            if (areNegDistance(first, second)) {
                errs() << "Dipendenza negativa trovata tra i loop\n";
                continue;
            }

            if (fuse(first, second, DT, LI)) {
                errs() << "FUSIONE RIUSCITA!\n";
                modified = true;
                topLevelLoops.erase(topLevelLoops.begin() + i + 1);
                --i;
            } else {
                errs() << "Fusion failed\n";
            }
        }

        errs() << "=== Function analysis complete ===\n\n";
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