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
#include "llvm/Analysis/CFGPrinter.h"

using namespace llvm;

namespace {

struct LoopFusionPass : PassInfoMixin<LoopFusionPass> {
    LoopInfo *LI;
    DominatorTree *DT;

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
        
        if (!Exit1 || !Pre2) return false;

        // caso 1: connessione diretta
        if (Exit1 == Pre2) {
            errs() << "-> rilevata connessione diretta\n";
            return true;
        }
        for (BasicBlock *Succ : successors(Exit1))
            if (Succ == Pre2) {
                errs() << "-> rilevata connessione diretta\n";
                return true;
            }
        
        // caso 2: pattern con guardia
        BasicBlock *Guard = nullptr;
        
        // cerca il blocco di guardia guardando i predecessori del preheader
        for (BasicBlock *Pred : predecessors(Pre1)) {
            // verifica che sia un branch condizionale
            if (BranchInst *Branch = dyn_cast<BranchInst>(Pred->getTerminator())) {
                if (!Branch->isConditional()) continue;
                
                // verifica che i successori portino ai due loop
                BasicBlock *Succ0 = Branch->getSuccessor(0);
                BasicBlock *Succ1 = Branch->getSuccessor(1);
                
                // controlla se un successore porta al primo loop e l'altro al secondo
                bool toL1_0 = (Succ0 == Pre1);
                bool toL1_1 = (Succ1 == Pre1);
                bool toL2_0 = (Succ0 == Pre2);
                bool toL2_1 = (Succ1 == Pre2);
                
                if ((toL1_0 && toL2_1) || (toL1_1 && toL2_0)) {
                    Guard = Pred;
                    break;
                }
            }
        }

        if (Guard) {
            errs() << "-> rilevato pattern con guardia\n";
            return true;
        }

        errs() << "-> nessuna connessione trovata\n";
        return false;
    }

    bool fuse(Loop *firstLoop, Loop *secondLoop, DominatorTree &domTree, LoopInfo &loopInfo) {
        return false;  // Fusion disabled for debugging
    }

    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
        LI = &AM.getResult<LoopAnalysis>(F);
        DT = &AM.getResult<DominatorTreeAnalysis>(F);

        SmallVector<Loop*, 8> Loops(LI->begin(), LI->end());
        bool Changed = false;

        errs() << "\n=== analizzo " << F.getName() << " ===\n";
        errs() << "trovati " << Loops.size() << " loop\n";

        // analizza coppie di loop adiacenti
        for (size_t i = 0; i + 1 < Loops.size(); ++i) {
            Loop *L1 = Loops[i];
            Loop *L2 = Loops[i + 1];

            if (!canFuse(L1, L2)) continue;
            
            if (!areLoopsConnected(L1, L2)) {
                errs() << "-> loop non connessi\n\n";
                continue;
            }

            // determina ordine corretto
            if (areLoopsDirectlyConnected(L2, L1)) {
                errs() << "-> scambio ordine loop\n";
                std::swap(L1, L2);
            }

            // prova a fondere i loop
            if (fuse(L1, L2, *DT, *LI)) {
                errs() << "-> fusione completata\n\n";
                Changed = true;
                Loops.erase(Loops.begin() + i + 1);
                --i;
            } else {
                errs() << "-> fusione fallita\n\n";
            }
        }

        errs() << "=== analisi completata ===\n\n";
        return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
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