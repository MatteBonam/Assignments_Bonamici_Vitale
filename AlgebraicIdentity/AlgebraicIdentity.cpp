//=============================================================================
// FILE:
//    AlgebraicIdentity.cpp
//
// DESCRIPTION:
//    Visits all functions in a module and prints their names. Strictly speaking, 
//    this is an analysis pass (i.e. //    the functions are not modified). However, 
//    in order to keep things simple there's no 'print' method here (every analysis 
//    pass should implement it).
//
// USAGE:
//    New PM
//      opt -load-pass-plugin=<path-to>libTestPass.so -passes="test-pass" `\`
//        -disable-output <input-llvm-file>
//
//
// License: MIT
//=============================================================================
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

//-----------------------------------------------------------------------------
// TestPass implementation
//-----------------------------------------------------------------------------
// No need to expose the internals of the pass to the outside world - keep
// everything in an anonymous namespace.
namespace {

bool runOnInstruction(Instruction &I) {

    if (I.getOpcode() == Instruction::Mul){
      outs() << "RILEVATA MUL " << "\n";
      Value *Operand1 = I.getOperand(0);
      Value *Operand2 = I.getOperand(1);
      if (ConstantInt *C = dyn_cast<ConstantInt>(Operand1)){
        outs() << "Opearand 1 -> " << C->getValue() << "\n";
        if(C->getValue() == 1){
          I.replaceAllUsesWith(Operand2);
        }
      }
      if(ConstantInt *C1 = dyn_cast<ConstantInt>(Operand2)){
        outs() << "Opearand 2 -> " << C1->getValue() << "\n";
          if(C1->getValue() == 1){
            I.replaceAllUsesWith(Operand1);
        }
      }
    } else if (I.getOpcode() == Instruction::Add){
      outs() << "RILEVATA ADD " << "\n";
      Value *Operand1 = I.getOperand(0);
      Value *Operand2 = I.getOperand(1);
      if (ConstantInt *C = dyn_cast<ConstantInt>(Operand1)){
        outs() << "Opearand 1 -> " << C->getValue() << "\n";
        if(C->getValue() == 0){
          I.replaceAllUsesWith(Operand2);
        }
      }
      if(ConstantInt *C1 = dyn_cast<ConstantInt>(Operand2)){
        outs() << "Opearand 2 -> " << C1->getValue() << "\n";
          if(C1->getValue() == 0){
            I.replaceAllUsesWith(Operand1);
        }
      }
    }
    //   Manipolazione delle istruzioni
    //Instruction *NewInst = BinaryOperator::Create(
    //    Instruction::Add, Inst1st.getOperand(0), Inst1st.getOperand(0));

    //NewInst->insertAfter(&Inst1st);
    // Si possono aggiornare le singole references separatamente?
    // Controlla la documentazione e prova a rispondere.
    //Inst1st.replaceAllUsesWith(NewInst);
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



// New PM implementation
struct AlgebraicIdentity: PassInfoMixin<AlgebraicIdentity> {
  // Main entry point, takes IR unit to run the pass on (&F) and the
  // corresponding pass manager (to be queried if need be)
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {

    bool Transformed = false;
  	for (auto Iter = F.begin(); Iter != F.end(); ++Iter) {
    if (runOnBasicBlock(*Iter)) {
      Transformed = true;
      }
    }
    errs() << Transformed << "\n";
  	return PreservedAnalyses::all();
}


  // Without isRequired returning true, this pass will be skipped for functions
  // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
  // all functions with optnone.
  static bool isRequired() { return true; }
};
} // namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getAlgebraicIdentityPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "AlgebraicIdentity", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "AlgebraicIdentity") {
                    FPM.addPass(TestPass());
                    return true;
                  }
                  return false;
                });
          }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize TestPass when added to the pass pipeline on the
// command line, i.e. via '-passes=test-pass'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getTestPassPluginInfo();
}
