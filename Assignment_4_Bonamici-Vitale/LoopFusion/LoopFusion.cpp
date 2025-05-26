#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

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

    bool areLoopsDirectlyConnected(Loop *firstLoop, Loop *secondLoop) {
        BasicBlock *Exit1 = firstLoop->getUniqueExitBlock();
        BasicBlock *Pre2 = secondLoop->getLoopPreheader();
        
        if (!Exit1 || !Pre2) {
            return false;
        }

        if (Exit1 == Pre2) {
            return true;
        }

        for (BasicBlock *Succ : successors(Exit1)) {
            if (Succ == Pre2) {
                return true;
            }
        }

        return false;
    }

    bool fuse(Loop *firstLoop, Loop *secondLoop, DominatorTree &domTree, LoopInfo &loopInfo) {
        return false;  // Fusion disabled for debugging
    }

    PreservedAnalyses run(Function &function, FunctionAnalysisManager &analysisManager) {
        errs() << "\n=== Starting function analysis: " << function.getName() << " ===\n";
        
        auto &loopInfo = analysisManager.getResult<LoopAnalysis>(function);
        auto &domTree = analysisManager.getResult<DominatorTreeAnalysis>(function);

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