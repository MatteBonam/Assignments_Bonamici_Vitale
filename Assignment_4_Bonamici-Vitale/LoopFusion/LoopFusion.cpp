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
        return firstLoop->getLoopLatch() && secondLoop->getLoopLatch() &&
               firstLoop->getUniqueExitBlock() && secondLoop->getUniqueExitBlock();
    }

    bool areLoopsConnected(Loop *firstLoop, Loop *secondLoop) {
        BasicBlock *firstLoopExit = firstLoop->getUniqueExitBlock();
        BasicBlock *secondLoopEntry = secondLoop->getLoopPreheader();

        if (!firstLoopExit || !secondLoopEntry)
            return false;

        SmallVector<BasicBlock*, 8> blocksToVisit;
        SmallPtrSet<BasicBlock*, 8> visitedBlocks;
        blocksToVisit.push_back(firstLoopExit);
        visitedBlocks.insert(firstLoopExit);

        while (!blocksToVisit.empty()) {
            BasicBlock *currentBlock = blocksToVisit.pop_back_val();
            if (currentBlock == secondLoopEntry)
                return true;

            for (BasicBlock *nextBlock : successors(currentBlock)) {
                if (!visitedBlocks.contains(nextBlock)) {
                    visitedBlocks.insert(nextBlock);
                    blocksToVisit.push_back(nextBlock);
                }
            }
        }

        return false;
    }

    bool fuse(Loop *firstLoop, Loop *secondLoop, DominatorTree &domTree, LoopInfo &loopInfo) {
        errs() << "Starting loop fusion...\n";
        
        BasicBlock *firstHeader = firstLoop->getHeader();
        BasicBlock *firstLatch = firstLoop->getLoopLatch();
        BasicBlock *firstExit = firstLoop->getUniqueExitBlock();
        BasicBlock *secondHeader = secondLoop->getHeader();
        BasicBlock *secondLatch = secondLoop->getLoopLatch();
        BasicBlock *secondExit = secondLoop->getUniqueExitBlock();
        BasicBlock *secondEntry = secondLoop->getLoopPreheader();

        if (!firstHeader || !firstLatch || !firstExit || !secondHeader || 
            !secondLatch || !secondExit || !secondEntry) {
            errs() << "Missing required blocks\n";
            return false;
        }

        SmallVector<BasicBlock*, 8> originalSecondLoopBlocks(secondLoop->blocks().begin(), 
                                                            secondLoop->blocks().end());
        
        ValueToValueMapTy valueMap;
        SmallVector<BasicBlock*, 8> clonedBlocks;

        for (BasicBlock *block : secondLoop->blocks()) {
            BasicBlock *clonedBlock = CloneBasicBlock(block, valueMap, ".fused", 
                                                     block->getParent());
            valueMap[block] = clonedBlock;
            clonedBlocks.push_back(clonedBlock);
        }

        for (BasicBlock *clonedBlock : clonedBlocks) {
            for (Instruction &inst : *clonedBlock) {
                RemapInstruction(&inst, valueMap, RF_NoModuleLevelChanges | 
                               RF_IgnoreMissingLocals);
            }
        }

        BasicBlock *clonedHeader = cast<BasicBlock>(valueMap[secondHeader]);
        BasicBlock *clonedExit = cast<BasicBlock>(valueMap[secondExit]);

        for (auto &inst : *clonedHeader) {
            if (auto *phiNode = dyn_cast<PHINode>(&inst)) {
                int entryIndex = phiNode->getBasicBlockIndex(secondEntry);
                if (entryIndex != -1) {
                    Value *incomingValue = phiNode->getIncomingValue(entryIndex);
                    phiNode->removeIncomingValue(entryIndex);
                    phiNode->addIncoming(incomingValue, firstLatch);
                }
            } else {
                break;
            }
        }

        if (BranchInst *branchInst = dyn_cast<BranchInst>(firstLatch->getTerminator())) {
            if (branchInst->isConditional()) {
                for (unsigned i = 0; i < branchInst->getNumSuccessors(); ++i) {
                    if (branchInst->getSuccessor(i) == firstExit) {
                        branchInst->setSuccessor(i, clonedHeader);
                        break;
                    }
                }
            } else {
                branchInst->replaceUsesOfWith(firstExit, clonedHeader);
            }
        }

        if (BranchInst *exitBranch = dyn_cast<BranchInst>(clonedExit->getTerminator())) {
            exitBranch->replaceUsesOfWith(secondExit, firstExit);
        }

        loopInfo.erase(secondLoop);
        
        if (secondEntry->use_empty()) {
            secondEntry->eraseFromParent();
        }

        for (BasicBlock *clonedBlock : clonedBlocks) {
            firstLoop->addBasicBlockToLoop(clonedBlock, loopInfo);
        }
        
        domTree.recalculate(*firstHeader->getParent());
        
        errs() << "Loop fusion completed successfully!\n";
        return true;
    }

    PreservedAnalyses run(Function &function, FunctionAnalysisManager &analysisManager) {
        errs() << "\n=== Starting function analysis: " << function.getName() << " ===\n";
        
        auto &loopInfo = analysisManager.getResult<LoopAnalysis>(function);
        auto &domTree = analysisManager.getResult<DominatorTreeAnalysis>(function);

        SmallVector<Loop*, 8> topLevelLoops(loopInfo.begin(), loopInfo.end());
        bool modified = false;

        errs() << "Found " << topLevelLoops.size() << " top-level loops\n";

        for (size_t i = 0; i + 1 < topLevelLoops.size(); ++i) {
            Loop *firstLoop = topLevelLoops[i];
            Loop *secondLoop = topLevelLoops[i + 1];

            errs() << "\nAnalyzing loop pair " << i << " and " << (i+1) << ":\n";

            if (!canFuse(firstLoop, secondLoop)) {
                errs() << "Loops cannot be fused: missing latch or exit block\n";
                continue;
            }

            if (!areLoopsConnected(firstLoop, secondLoop)) {
                errs() << "Loops are not directly connected in CFG\n";
                continue;
            }

            errs() << "Connected loops found! Attempting fusion...\n";
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