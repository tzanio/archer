//===-- InstrumentParallel.cpp - Archer race detector -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of Archer, an OpenMP race detector.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Instrumentation.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/ProfileData/InstrProf.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "archer/LinkAllPasses.h"

using namespace llvm;

extern "C" unsigned long long get_next_id();

#define DEBUG_TYPE "archer"

namespace {

struct InstrumentParallel : public FunctionPass {
  InstrumentParallel() : FunctionPass(ID) {}
  StringRef getPassName() const override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnFunction(Function &F) override;
  bool doInitialization(Module &M) override;
  static char ID;  // Pass identification, replacement for typeid.

private:
  void setMetadata(Instruction *Inst, const char *name, const char *description);
};
}  // namespace

char InstrumentParallel::ID = 0;
INITIALIZE_PASS_BEGIN(
    InstrumentParallel, "archer-sbl",
    "InstrumentParallel: instrument parallel functions.",
    false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_END(
    InstrumentParallel, "archer-sbl",
    "InstrumentParallel: instrument parallel functions.",
    false, false)

StringRef InstrumentParallel::getPassName() const {
  return "InstrumentParallel";
}

void InstrumentParallel::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<AAResultsWrapperPass>();
}

Pass *llvm::createInstrumentParallelPass() {
  return new InstrumentParallel();
}

bool InstrumentParallel::doInitialization(Module &M) {
  return true;
}

void InstrumentParallel::setMetadata(Instruction *Inst, const char *name, const char *description) {
  LLVMContext& C = Inst->getContext();
  MDNode* N = MDNode::get(C, MDString::get(C, description));
  Inst->setMetadata(name, N);
}

bool InstrumentParallel::runOnFunction(Function &F) {
  llvm::GlobalVariable *ompStatusGlobal = NULL;
  Module *M = F.getParent();
  StringRef functionName = F.getName();

  ConstantInt *Zero = ConstantInt::get(Type::getInt32Ty(M->getContext()), 0);
  ConstantInt *One = ConstantInt::get(Type::getInt32Ty(M->getContext()), 1);

  if(functionName.compare("main") == 0) {
    ompStatusGlobal = M->getNamedGlobal("__swordomp_status__");
    if(!ompStatusGlobal) {
      IntegerType *Int32Ty = IntegerType::getInt32Ty(M->getContext());
      ompStatusGlobal =
        new llvm::GlobalVariable(*M, Int32Ty, false,
                                 llvm::GlobalValue::CommonLinkage,
                                 Zero, "__swordomp_status__", NULL,
                                 GlobalVariable::GeneralDynamicTLSModel,
                                 0, false);
    }
    return true;
  }

  if(functionName.endswith("_dtor") ||
     functionName.endswith("__swordomp__") ||
     functionName.endswith("__clang_call_terminate")) {
    return true;
  }

  ompStatusGlobal = M->getNamedGlobal("__swordomp_status__");
  if(!ompStatusGlobal) {
    IntegerType *Int32Ty = IntegerType::getInt32Ty(M->getContext());
    ompStatusGlobal =
      new llvm::GlobalVariable(*M, Int32Ty, false,
                               llvm::GlobalValue::ExternalLinkage,
                               0, "__swordomp_status__", NULL,
                               GlobalVariable::GeneralDynamicTLSModel,
                               0, true);
  }

  if(functionName.startswith(".omp")) {
    // Increment of __swordomp_status__
    Instruction *entryBBI = &F.getEntryBlock().front();
    LoadInst *loadInc = new LoadInst(ompStatusGlobal, "loadIncOmpStatus", false, entryBBI);
    loadInc->setAlignment(4);
    setMetadata(loadInc, "swordomp.ompstatus", "SwordRT Instrumentation");
    Instruction *inc = BinaryOperator::Create (BinaryOperator::Add,
                                               loadInc, One,
                                               "incOmpStatus",
                                               entryBBI);
    setMetadata(loadInc, "swordrt.ompstatus", "SwordRT Instrumentation");
    StoreInst *storeInc = new StoreInst(inc, ompStatusGlobal, entryBBI);
    storeInc->setAlignment(4);
    setMetadata(storeInc, "swordrt.ompstatus", "SwordRT Instrumentation");

    // Decrement of __swordomp_status__
    Instruction *exitBBI = F.back().getTerminator();
    if(exitBBI) {
      LoadInst *loadDec = new LoadInst(ompStatusGlobal, "loadDecOmpStatus", false, exitBBI);
      loadDec->setAlignment(4);
      setMetadata(loadDec, "swordrt.ompstatus", "SwordRT Instrumentation");
      Instruction *dec = BinaryOperator::Create (BinaryOperator::Sub,
                                                 loadDec, One,
                                                 "decOmpStatus",
                                                 exitBBI);
      StoreInst *storeDec = new StoreInst(dec, ompStatusGlobal, exitBBI);
      storeDec->setAlignment(4);
      setMetadata(storeDec, "swordrt.ompstatus", "SwordRT Instrumentation");
    } else {
      report_fatal_error("Broken function found, compilation aborted!");
    }
  } else {
    ValueToValueMapTy VMap;
    Function *new_function = CloneFunction(&F, VMap);
    new_function->setName(functionName + "__swordomp__");
    Function::ArgumentListType::iterator it = F.getArgumentList().begin();                                                       Function::ArgumentListType::iterator end = F.getArgumentList().end();
    std::vector<Value*> args;
    while (it != end) {
      Argument *Args = &(*it);
      args.push_back(Args);
      it++;                                                                                                                      }

    // Removing SanitizeThread attribute so the sequential functions
    // won't be instrumented
    F.removeFnAttr(llvm::Attribute::SanitizeThread);

    // Instruction *firstEntryBBI = &F.getEntryBlock().front();
    Instruction *firstEntryBBI = NULL;
    Instruction *firstEntryBBDI = NULL;
    // for (auto &BB : F) {
    for (auto &Inst : F.getEntryBlock()) {
      if(Inst.getDebugLoc()) {
        firstEntryBBDI = &Inst;
        break;
      }
    }
    firstEntryBBI = &F.getEntryBlock().front();
    // }
    if(!firstEntryBBI || !firstEntryBBI)
      report_fatal_error("No instructions with debug information!");

    LoadInst *loadOmpStatus = new LoadInst(ompStatusGlobal, "loadOmpStatus", false, firstEntryBBI);
    Instruction *CondInst = new ICmpInst(firstEntryBBI, ICmpInst::ICMP_EQ, loadOmpStatus, One, "__swordomp__cond");

    BasicBlock *newEntryBB = F.getEntryBlock().splitBasicBlock(firstEntryBBI, "__swordomp__entry");
    F.getEntryBlock().back().eraseFromParent();
    BasicBlock *swordThenBB = BasicBlock::Create(M->getContext(), "__swordomp__if.then", &F);
    BranchInst::Create(swordThenBB, newEntryBB, CondInst, &F.getEntryBlock());

    // For now we assing the debug loc of the first instruction of the
    // cloned function
    if(new_function->getReturnType()->isVoidTy()) {
      CallInst *parallelCall = CallInst::Create(new_function, args, "", swordThenBB);
      parallelCall->setDebugLoc(firstEntryBBDI->getDebugLoc());
      ReturnInst::Create(M->getContext(), nullptr, swordThenBB);
    } else {
      CallInst *parallelCall = CallInst::Create(new_function, args, functionName + "__swordomp__", swordThenBB);
      parallelCall->setDebugLoc(firstEntryBBDI->getDebugLoc());
      ReturnInst::Create(M->getContext(), parallelCall, swordThenBB);
    }
 }

  return true;
}

static void registerInstrumentParallelPass(const PassManagerBuilder &, llvm::legacy::PassManagerBase &PM) {
  PM.add(new InstrumentParallel());
}

static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible, registerInstrumentParallelPass);