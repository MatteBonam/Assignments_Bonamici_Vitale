#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
  bool optimizeWithShiftAddSub(Value *operand, int constantValue, Instruction &I) {
    //se fosse 
    //calcolo il log del valore e 2^ris
    int p = log2(constantValue);
    int nearestPowerOf2 = 1 << p; //him
    // differenza (nel nostro esempio -> 64-15)
    int diff = constantValue - nearestPowerOf2;

    if ((constantValue & (constantValue - 1)) == 0) {
        Instruction *newInstr = BinaryOperator::CreateShl(operand, ConstantInt::get(operand->getType(), p), "shiftLeft", &I);
        I.replaceAllUsesWith(newInstr);  
        I.eraseFromParent();            
        return true;
    }

    if (diff != 0 && (diff & (diff - 1)) == 0) { 
         int q = log2(abs(diff));

         if (diff > 0) {
            // tipo 10 = 8 + 2 sostituisco [(operand << p) + (operand << q)]
            // ==> operand << (2^)3 + operand << (2^)1
            Instruction *shiftP = BinaryOperator::CreateShl(operand, ConstantInt::get(operand->getType(), p), "shiftP", &I);
            Instruction *shiftQ = BinaryOperator::CreateShl(operand, ConstantInt::get(operand->getType(), q), "shiftQ", &I);
            Instruction *add = BinaryOperator::CreateAdd(shiftP, shiftQ, "add", &I);
            I.replaceAllUsesWith(add);
        } else {
            // tipo 48 (log2(48) = 5.5) ==> 64 - 48
            // (operand << p) - (operand << q)
            Instruction *shiftP = BinaryOperator::CreateShl(operand, ConstantInt::get(operand->getType(), p), "shiftP", &I);
            Instruction *shiftQ = BinaryOperator::CreateShl(operand, ConstantInt::get(operand->getType(), q), "shiftQ", &I);
            Instruction *sub = BinaryOperator::CreateSub(shiftP, shiftQ, "sub", &I);
            I.replaceAllUsesWith(sub); 
            
        }
        //I.eraseFromParent();
        return true;
    }
    return false; 
  }
  bool runOnInstruction(Instruction &I) {
    outs() << "ISTRUZIONE: " << I << "\n";

    if (I.getNumOperands() < 2) return false;

    Value *Op0 = I.getOperand(0);
    Value *Op1 = I.getOperand(1);


    if (I.getOpcode() == Instruction::Mul) {
        if (ConstantInt *C = dyn_cast<ConstantInt>(Op0)) {
            int constantValue = C->getSExtValue();
            if (optimizeWithShiftAddSub(Op1, constantValue, I)) {
                outs() << "MULT PER " << constantValue << " OTTIMIZZATA\n";
            }
        } else if (ConstantInt *C = dyn_cast<ConstantInt>(Op1)) {
            int constantValue = C->getSExtValue();
            if (optimizeWithShiftAddSub(Op0, constantValue, I)) {
                outs() << "MULT PER " << constantValue << " OTTIMIZZATA \n";
            }
        }
    }
    return true;
}


bool runOnBasicBlock(BasicBlock &B) {
    
    // Preleviamo le prime due istruzioni del BB
    for (auto Iter = B.begin(); Iter != B.end(); ++Iter) {
    if (runOnInstruction(*Iter)) {
      }
    }
    // L'indirizzo della prima istruzione deve essere uguale a quello del 
    // primo operando della seconda istruzione (per costruzione dell'esempio)

    
    return true;
  }

struct StrengthReduction: PassInfoMixin<StrengthReduction> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    outs() << "CURR FUNCTION: " << F.getName() << "\n";
    bool Transformed = false;
    
    for (auto Iter = F.begin(); Iter != F.end(); ++Iter) {
        if (runOnBasicBlock(*Iter)) {
          Transformed = true;
        }
    }
    errs() << Transformed << "\n";

    return PreservedAnalyses::all();
}

  static bool isRequired() { return true; }
};
} 

llvm::PassPluginLibraryInfo getStrengthReductionPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "StrengthReduction", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "StrengthReduction") {
                    FPM.addPass(StrengthReduction());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getStrengthReductionPluginInfo();
}
