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
    bool canFuse(Loop *firstLoop, Loop *secondLoop) {
        // Check all required blocks exist
        BasicBlock *latch1 = firstLoop->getLoopLatch();
        BasicBlock *latch2 = secondLoop->getLoopLatch();
        BasicBlock *exit1 = firstLoop->getUniqueExitBlock();
        BasicBlock *exit2 = secondLoop->getUniqueExitBlock();
        BasicBlock *header1 = firstLoop->getHeader();
        BasicBlock *header2 = secondLoop->getHeader();
        BasicBlock *pre1 = firstLoop->getLoopPreheader();
        BasicBlock *pre2 = secondLoop->getLoopPreheader();

        return latch1 && latch2 && exit1 && exit2 && 
               header1 && header2 && pre1 && pre2;
    }

    bool areLoopsConnected(Loop *L1, Loop *L2) {
        // Try both directions since we don't know the order in the CFG
        return areLoopsDirectlyConnected(L1, L2) || areLoopsDirectlyConnected(L2, L1);
    }

    bool areLoopsDirectlyConnected(Loop *L1, Loop *L2) {
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
        
        // cerca il blocco di guardia guardando i predecessori del preheader
        for (BasicBlock *Pred : predecessors(Pre1)) {
            errs() << "  controllo predecessore del preheader: " << Pred->getName() << "\n";
            
            // verifica che sia un branch condizionale
            if (BranchInst *Branch = dyn_cast<BranchInst>(Pred->getTerminator())) {
                errs() << "    trovato branch: " << *Branch << "\n";
                
                if (!Branch->isConditional()) {
                    errs() << "    non è condizionale\n";
                    continue;
                }
                
                // verifica che i successori portino ai due loop
                BasicBlock *Succ0 = Branch->getSuccessor(0);
                BasicBlock *Succ1 = Branch->getSuccessor(1);
                
                errs() << "    successore 0: " << Succ0->getName() << "\n";
                errs() << "    successore 1: " << Succ1->getName() << "\n";
                
                // controlla se un successore porta al primo loop e l'altro al secondo
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

    bool areControlFlowEquivalent(Loop *L0, Loop *L1, DominatorTree &DT, PostDominatorTree &PDT) {
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
        
        errs() << "flow Equivalence check:\n";
        errs() << "  L0 domina L1: " << (L0DominatesL1 ? "si" : "no") << "\n";
        errs() << "  L1 post-domina L0: " << (L1PostDominatesL0 ? "si" : "no") << "\n";
        
        if (L0DominatesL1 && L1PostDominatesL0) {
            errs() << "  -> Loop sono control flow equivalent!\n";
            return true;
        }
        return false;
    }

    bool fuse(Loop *firstLoop, Loop *secondLoop, DominatorTree &domTree, LoopInfo &loopInfo) {
        return false;  
    }

    PreservedAnalyses run(Function &function, FunctionAnalysisManager &analysisManager) {
        errs() << "\n=== Starting function analysis: " << function.getName() << " ===\n";
        
        auto &loopInfo = analysisManager.getResult<LoopAnalysis>(function);
        auto &domTree = analysisManager.getResult<DominatorTreeAnalysis>(function);
        auto &postDomTree = analysisManager.getResult<PostDominatorTreeAnalysis>(function);

        SmallVector<Loop*, 8> topLevelLoops(loopInfo.begin(), loopInfo.end());
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

            if (!areLoopsConnected(L1, L2)) {
                errs() << "Loops are not directly connected in CFG\n";
                continue;
            }

            errs() << "Trovati loop adiacenti!\n";
            
            //Troviamo ordine loops
            Loop *firstLoop = L1;
            Loop *secondLoop = L2;
            if (areLoopsDirectlyConnected(L2, L1)) {
                errs() << "Swappo ordine loops!\n";
                std::swap(firstLoop, secondLoop);
            }

            // Verifica control flow equivalence dopo aver determinato l'ordine
            if (!areControlFlowEquivalent(firstLoop, secondLoop, domTree, postDomTree)) {
                errs() << "Loops are not control flow equivalent\n";
                continue;
            }

            if (fuse(firstLoop, secondLoop, domTree, loopInfo)) {
                errs() << "Fusion successful!\n";
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