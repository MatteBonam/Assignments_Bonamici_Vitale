#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
  int nearestPowerOfTwo(int n) {
    if (n == 0) return 0;
    int lowerPower = 1 << (int)log2(n); 
    int upperPower = lowerPower << 1;   
    return (n - lowerPower) <= (upperPower - n) ? lowerPower : upperPower;
  }

  bool isCloserToPowerOfTwo(int a, int b) {
    int nearestA = nearestPowerOfTwo(a);
    int nearestB = nearestPowerOfTwo(b);
    return abs(a - nearestA) < abs(b - nearestB);
  }


  bool optimizeWithShiftAddSub(Value *operand, int constantValue, Instruction &I) {
    //se gia potenza di 2..
    outs() << "RUNNO OPT CON CV --> " << constantValue << "\n";
    
    int p = std::round(log2(constantValue));
    int nearestPowerOf2 = nearestPowerOfTwo(constantValue);
    // differenza (nel nostro esempio -> 48 = 64-16)
    int diff = constantValue - nearestPowerOf2;

    if ((constantValue & (constantValue - 1)) == 0) {
      Instruction *newInstr = BinaryOperator::CreateShl(operand, ConstantInt::get(operand->getType(), p), "shiftLeft");
      newInstr->insertAfter(&I);   
      I.replaceAllUsesWith(newInstr);        
      return true;
  }  
    
    if (diff != 0 && (abs(diff) & (abs(diff) - 1)) == 0) { 
        int q = log2(abs(diff));
        //calcolo il log del valore e 2^ris
    
    if (diff > 0) {
      // tipo 48 (log2(48) = 5.5 -> 6) ==> 64 - 16
      // (operand << p) - (operand << q)
      Instruction *add;
      Instruction *shiftP = BinaryOperator::CreateShl(operand, ConstantInt::get(operand->getType(), p), "shiftP");
      shiftP->insertAfter(&I);      
 
      if(q != 0){
          Instruction *shiftQ = BinaryOperator::CreateShl(operand, ConstantInt::get(operand->getType(), q), "shiftQ");
          add = BinaryOperator::CreateAdd(shiftP, shiftQ, "add");
          shiftQ->insertAfter(&I);
      } else {
          add = BinaryOperator::CreateAdd(shiftP, operand, "add");
        }
        add->insertAfter(shiftP);
        // 2^4 - 2^0 = 15
        I.replaceAllUsesWith(add); 
      } else {
        // tipo 48 (log2(48) = 5.5 -> 6) ==> 64 - 16
        // (operand << p) - (operand << q)
        Instruction *sub;
        Instruction *shiftP = BinaryOperator::CreateShl(operand, ConstantInt::get(operand->getType(), p), "shiftP");
        shiftP->insertAfter(&I);      
   
        if(q != 0){
            Instruction *shiftQ = BinaryOperator::CreateShl(operand, ConstantInt::get(operand->getType(), q), "shiftQ");
            sub = BinaryOperator::CreateSub(shiftP, shiftQ, "sub");
            shiftQ->insertAfter(&I);
        } else {
            sub = BinaryOperator::CreateSub(shiftP, operand, "sub");
          }
          sub->insertAfter(shiftP);
          // 2^4 - 2^0 = 15
          I.replaceAllUsesWith(sub); 
        }
    
      return true;
    }

    //se la diff non e' multiplo di 2 allora niente opt
    return false; 
  }

  bool runOnInstruction(Instruction &I) {
    outs() << "ISTRUZIONE: " << I << "\n";

    //importante return faila
    if (I.getNumOperands() < 2) return false;

    Value *Op0 = I.getOperand(0);
    Value *Op1 = I.getOperand(1);

    if (I.getOpcode() == Instruction::Mul) {
      ConstantInt *C0 = dyn_cast<ConstantInt>(Op0);
      ConstantInt *C1 = dyn_cast<ConstantInt>(Op1);
  
      if (C0 && C1) {
          int constantValue0 = C0->getSExtValue();
          int constantValue1 = C1->getSExtValue();
  
          if (isCloserToPowerOfTwo(constantValue0, constantValue1)) {
              if (optimizeWithShiftAddSub(Op1, constantValue0, I)) {
                  outs() << "MULT PER " << constantValue0 << " OTTIMIZZATA\n";
              }
          } else {
              if (optimizeWithShiftAddSub(Op0, constantValue1, I)) {
                  outs() << "MULT PER " << constantValue1 << " OTTIMIZZATA\n";
              }
          }
      } else if (C0) {
          int constantValue = C0->getSExtValue(); 
          if (optimizeWithShiftAddSub(Op1, constantValue, I)) {
              outs() << "MULT PER " << constantValue << " OTTIMIZZATA\n";
          }
      } else if (C1) {
          int constantValue = C1->getSExtValue();  
          if (optimizeWithShiftAddSub(Op0, constantValue, I)) {
              outs() << "MULT PER " << constantValue << " OTTIMIZZATA \n";
          }
      }
    }
    
    // if (I.getOpcode() == Instruction::Div || I.getOpcode() == Instruction::SDiv){
    //   continue;
    // }
    return true;


    //   Manipolazione delle istruzioni
    //Instruction *NewInst = BinaryOperator::Create(
    //    Instruction::Add, Inst1st.getOperand(0), Inst1st.getOperand(0));

    //NewInst->insertAfter(&Inst1st);
    // Si possono aggiornare le singole references separatamente?
    // Controlla la documentazione e prova a rispondere.
    //Inst1st.replaceAllUsesWith(NewInst);
}


bool runOnBasicBlock(BasicBlock &B) {
    // Preleviamo le prime due istruzioni del BB
    for (auto Iter = B.begin(); Iter != B.end(); ++Iter) {
      if (runOnInstruction(*Iter)) {}
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
