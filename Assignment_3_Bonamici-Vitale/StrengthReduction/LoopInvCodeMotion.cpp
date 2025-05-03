#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "math.h"

using namespace llvm;

namespace {

struct LoopCodeM: PassInfoMixin<LoopCodeM> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopInfo &LI = AM.getResult<LoopAnalysis>(F);

    for (Loop::block_iterator BI = L->block_begin(); BI != L->block_end(); ++BI) {}

    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);

    for (auto *DTN : depth_first(DT.getRootNode())) {}
    for (auto *DTN : breadth_first(DT.getRootNode())) {}  //#include "llvm/ADT/BreadthFirstIterator.h"

    return PreservedAnalyses::all();
}

  static bool isRequired() { return true; }
};
} 

llvm::PassPluginLibraryInfo getLoopCodeMPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "LoopCodeM", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "s-r") {
                    FPM.addPass(LoopCodeM());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getLoopCodeMPluginInfo();
}
