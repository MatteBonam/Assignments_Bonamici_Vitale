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

    bool fuse(Loop *firstLoop, Loop *secondLoop, DominatorTree &domTree, LoopInfo &loopInfo) {
        return false;  
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

            if (fuse(first, second, DT, LI)) {
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