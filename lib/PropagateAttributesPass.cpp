//
//
//

#include "llvm/Pass.h"
// using llvm::RegisterPass

#include "llvm/IR/LegacyPassManager.h"
// using llvm::PassManagerBase

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/Attributes.h"
// using llvm::Attribute
// using llvm::AttrBuilder

#include "llvm/Analysis/CallGraph.h"
// using llvm::CallGraph

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// using llvm::PassManagerBuilder
// using llvm::RegisterStandardPasses

#include "llvm/ADT/SmallPtrSet.h"
// using llvm::SmallPtrSet

#include "llvm/Support/raw_ostream.h"
// using llvm::raw_ostream

#include "llvm/Support/Debug.h"
// using DEBUG macro
// using llvm::dbgs

#include <algorithm>
// using std::for_each

#include "PropagateAttributesPass.hpp"

#define DEBUG_TYPE "propagate_attributes"

#ifndef NDEBUG
#define PLUGIN_OUT llvm::outs()
//#define PLUGIN_OUT llvm::nulls()

// convenience macro when building against a NDEBUG LLVM
#undef DEBUG
#define DEBUG(X)                                                               \
  do {                                                                         \
    X;                                                                         \
  } while (0);
#else // NDEBUG
#define PLUGIN_OUT llvm::dbgs()
#endif // NDEBUG

#define PLUGIN_ERR llvm::errs()

// plugin registration for opt

char PropagateAttributesPass::ID = 0;
static llvm::RegisterPass<PropagateAttributesPass>
    X("propagate-attributes", "propagate function attributes pass", false,
      false);

// plugin registration for clang

// the solution was at the bottom of the header file
// 'llvm/Transforms/IPO/PassManagerBuilder.h'
// create a static free-floating callback that uses the legacy pass manager to
// add an instance of this pass and a static instance of the
// RegisterStandardPasses class

static void
registerPropagateAttributesPass(const llvm::PassManagerBuilder &Builder,
                                llvm::legacy::PassManagerBase &PM) {
  PM.add(new PropagateAttributesPass());

  return;
}

static llvm::RegisterStandardPasses RegisterPropagateAttributesPass(
    llvm::PassManagerBuilder::EP_EarlyAsPossible,
    registerPropagateAttributesPass);

//

namespace {

PropagateAttributesPass::PropagateAttributesPass() : llvm::ModulePass(ID) {
  m_CustomAttributes.insert("icsa.dynapar.performs-io");

  // builtin attributes
  m_AttrBuilder.addAttribute(llvm::Attribute::NoReturn);

  std::for_each(std::begin(m_CustomAttributes), std::end(m_CustomAttributes),
                [&](auto &e) { m_AttrBuilder.addAttribute(e); });

  return;
}

void PropagateAttributesPass::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  AU.addRequired<llvm::CallGraphWrapperPass>();

  return;
}

bool PropagateAttributesPass::runOnModule(llvm::Module &M) {
  llvm::CallGraph &CG =
      getAnalysis<llvm::CallGraphWrapperPass>().getCallGraph();

  return false;
}

} // namespace unnamed end
