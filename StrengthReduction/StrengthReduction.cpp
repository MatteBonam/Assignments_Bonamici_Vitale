#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

  bool runOnInstruction(Instruction &I) {
    outs() << "ISTRUZIONE: " << I << "\n";

    bool instructionModified = false;

    // Controlla se l'istruzione ha almeno due operandi
    if (I.getNumOperands() >= 2) {
        Value *Op0 = I.getOperand(0);
        Value *Op1 = I.getOperand(1);

        if (I.getOpcode() == Instruction::Mul) {
            if (ConstantInt *C = dyn_cast<ConstantInt>(Op0)) {
                if (C->getSExtValue() == 15) {
                    outs() << "MULT PER 15 RILEVATA, SOSTITUISCO "
                           << I << " CON SHIFT DI 4 " << *Op1 << "\n";
                    Value *shiftedOp0 = BinaryOperator::CreateShl(Op0, ConstantInt::get(Op0->getType(), 4), "", &I);
                    Value *result = BinaryOperator::CreateSub(shiftedOp0, Op0, "", &I);
                    I.replaceAllUsesWith(result);
                    instructionModified = true;
                }
            } else if (ConstantInt *C = dyn_cast<ConstantInt>(Op1)) {
                if (C->getSExtValue() == 15) {
                    outs() << "MULT PER 15 RILEVATA, SOSTITUISCO "
                           << I << " CON SHIFT LEFT DI 4 " << *Op1 << "\n";
                    Value *shiftedOp1 = BinaryOperator::CreateShl(Op1, ConstantInt::get(Op1->getType(), 4), "", &I);
                    Value *result = BinaryOperator::CreateSub(shiftedOp1, Op1, "", &I);
                    I.replaceAllUsesWith(result);
                    instructionModified = true;
                }
            }
        } else if (I.getOpcode() == Instruction::SDiv) {
            if (ConstantInt *C = dyn_cast<ConstantInt>(Op1)) {
                if (C->getSExtValue() == 8) {
                    outs() << "DIV PER 8 RILEVATA, SOSTITUISCO "
                           << I << " CON SHIFT RIGHT DI 3 " << *Op0 << "\n";
                    Value *result = BinaryOperator::CreateLShr(Op0, ConstantInt::get(Op1->getType(), 3), "", &I);
                    I.replaceAllUsesWith(result);
                    instructionModified = true;
                }
            }
        }
    }

    return instructionModified;
}


bool runOnBasicBlock(BasicBlock &B) {
    outs() << "CURR BLOCK: " << B.getName() << "\n";
    bool blockModified = false;
    
    std::vector<Instruction*> instructionsToErase;
    for (auto Iter = B.begin(); Iter != B.end(); ++Iter) {
        if (runOnInstruction(*Iter)) {
            instructionsToErase.push_back(&*Iter);
            blockModified = true;
        }
    }
    for (auto *I : instructionsToErase) {
        I->eraseFromParent();
    }

    return blockModified;
}

struct TestPass: PassInfoMixin<TestPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    outs() << "CURR FUNCTION: " << F.getName() << "\n";
    
    bool functionModified = false;
    for (auto Iter = F.begin(); Iter != F.end(); ++Iter) {
        if (runOnBasicBlock(*Iter)) {
            functionModified = true;
        }
    }
    
    outs() << "STATO FUNZIONE: " 
           << (functionModified ? "MODIFICATA" : "UGUALE") << "\n\n\n";
  	
    return PreservedAnalyses::all();
}

  static bool isRequired() { return true; }
};
} 

llvm::PassPluginLibraryInfo getTestPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "TestPass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "diobestia") {
                    FPM.addPass(TestPass());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getTestPassPluginInfo();
}