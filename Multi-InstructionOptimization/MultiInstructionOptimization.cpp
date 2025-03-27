//=============================================================================
// FILE:
//    MultiInstructionOptimization.cpp
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

void runOnBinaryOperator(BinaryOperator *BO1, BinaryOperator *BO2)
{
  //estraggo gli operandi
  Value *Op1 = BO1->getOperand(0);
  Value *Op2 = BO1->getOperand(1);
  Value *IOp1 = BO2->getOperand(0);
  Value *IOp2 = BO2->getOperand(1);
  //se la mia istruzione e' una add, cerco una sub con un operando uguale ad uno dei miei operandi, 
  //essendo che uno degli operandi sono io, solo uno dei 2 puo' essere uguale ad uno dei miei operandi
  if (BO1->getOpcode() == Instruction::Add && BO2->getOpcode() == Instruction::Sub)
  {
    outs() << "Sono: "  << *BO2 << " con operandi: " << *IOp1 << ' ' << *IOp2 << "\n";
    if (ConstantInt *IC1 = dyn_cast<ConstantInt>(IOp1))
    {
      if (ConstantInt *C1 = dyn_cast<ConstantInt>(Op1))
      {
        if (IC1->getValue() - C1->getValue() == 0)
        {
          outs() << "Rimpiazzato con: " << *Op2 << "\n";
          BO2->replaceAllUsesWith(Op2);
        }
      }else
      if (ConstantInt *C2 = dyn_cast<ConstantInt>(Op2))
      {
        if (IC1->getValue() - C2->getValue() == 0)
        {
          outs() << "Rimpiazzato con: " << *Op1 << "\n";
          BO2->replaceAllUsesWith(Op1);
        }
      }
    }
    if(ConstantInt *IC2 = dyn_cast<ConstantInt>(IOp2))
    {
      if (ConstantInt *C1 = dyn_cast<ConstantInt>(Op1))
      {
        if (IC2->getValue() - C1->getValue() == 0)
        {
          outs() << "Rimpiazzato con: " << *Op2 << "\n";
          BO2->replaceAllUsesWith(Op2);
        }
      }else
      if (ConstantInt *C2 = dyn_cast<ConstantInt>(Op2))
      {
        if (IC2->getValue() - C2->getValue() == 0)
        {
          outs() << "Rimpiazzato con: " << *Op1 << "\n";
          BO2->replaceAllUsesWith(Op1);
        }
      }
    }
  }
  //se la mia istruzione e' una add, cerco una add con un operando uguale al negativo di uno dei miei operandi, 
  //essendo che uno degli operandi sono io, solo uno dei 2 puo' essere uguale al negativo di uno dei miei operandi
  if (BO1->getOpcode() == Instruction::Add && BO2->getOpcode() == Instruction::Add)
  {
    outs() << "Sono: "  << *BO2 << " con operandi: " << *IOp1 << ' ' << *IOp2 << "\n";
    if (ConstantInt *IC1 = dyn_cast<ConstantInt>(IOp1))
    {
      if (ConstantInt *C1 = dyn_cast<ConstantInt>(Op1))
      {
        if (IC1->getValue() + C1->getValue() == 0)
        {
          outs() << "Rimpiazzato con: " << *Op2 << "\n";
          BO2->replaceAllUsesWith(Op2);
        }
      }else
      if (ConstantInt *C2 = dyn_cast<ConstantInt>(Op2))
      {
        if (IC1->getValue() + C2->getValue() == 0)
        {
          outs() << "Rimpiazzato con: " << *Op1 << "\n";
          BO2->replaceAllUsesWith(Op1);
        }
      }
    }
    if(ConstantInt *IC2 = dyn_cast<ConstantInt>(IOp2))
    {
      if (ConstantInt *C1 = dyn_cast<ConstantInt>(Op1))
      {
        if (IC2->getValue() + C1->getValue() == 0)
        {
          outs() << "Rimpiazzato con: " << *Op2 << "\n";
          BO2->replaceAllUsesWith(Op2);
        }
      }else
      if (ConstantInt *C2 = dyn_cast<ConstantInt>(Op2))
      {
        if (IC2->getValue() + C2->getValue() == 0)
        {
          outs() << "Rimpiazzato con: " << *Op1 << "\n";
          BO2->replaceAllUsesWith(Op1);
        }
      }
    }
  }
  //se la mia istruzione e' una mul, cerco una div con un operando uguale a uno dei miei operandi,
  //e nella seconda posizione, per assicurarmi che il prodotto sia diviso per lo stesso numero della moltiplicazione
  if (BO1->getOpcode() == Instruction::Mul && BO2->getOpcode() == Instruction::SDiv)
  {
    outs() << "Sono: "  << *BO2 << " con operandi: " << *IOp1 << ' ' << *IOp2 << "\n";
    if (ConstantInt *IC2 = dyn_cast<ConstantInt>(IOp2))
    {
      if (ConstantInt *C1 = dyn_cast<ConstantInt>(Op1))
      {
        if (IC2->getValue() == C1->getValue())
        {
          outs() << "Rimpiazzato con: " << *Op2 << "\n";
          BO2->replaceAllUsesWith(Op2);
        }
      }else
      if (ConstantInt *C2 = dyn_cast<ConstantInt>(Op2))
      {
        if (IC2->getValue() == C2->getValue())
        {
          outs() << "Rimpiazzato con: " << *Op1 << "\n";
          BO2->replaceAllUsesWith(Op1);
        }
      }
    }
  }
}

bool runOnBasicBlock(BasicBlock &B) {

    // Preleviamo le prime due istruzioni del BB
    for (auto Iter = B.begin(); Iter != B.end(); ++Iter) {
      // Controllo che sia un' operazione binaria
      if (BinaryOperator *BI = dyn_cast<BinaryOperator>(Iter)) {
        for (auto Ite = BI->use_begin(); Ite != BI->use_end(); ++Ite) {
          //controllo tutti gli usi dell'espressione
            if(BinaryOperator *I = dyn_cast<BinaryOperator>(Ite->getUser()))
            {
              runOnBinaryOperator(BI, I);
              outs() << "Io sono: " << *I << "\n";
            }
          }
        }
      }
    return true;
  }



// New PM implementation
struct MultiInstructionOptimization: PassInfoMixin<MultiInstructionOptimization> {
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
llvm::PassPluginLibraryInfo getMultiInstructionOptimizationPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MultiInstructionOptimization", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "m-i-o") {
                    FPM.addPass(MultiInstructionOptimization());
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
  return getMultiInstructionOptimizationPluginInfo();
}
