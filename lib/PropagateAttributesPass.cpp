//
//
//

#include "PropagateAttributesPass.hpp"

#include "llvm/Pass.h"
// using llvm::RegisterPass

#include "llvm/ADT/SmallPtrSet.h"
// using llvm::SmallPtrSet

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/Attributes.h"
// using llvm::Attribute
// using llvm::AttrBuilder

#include "llvm/IR/LegacyPassManager.h"
// using llvm::PassManagerBase

#include "llvm/Analysis/CallGraph.h"
// using llvm::CallGraph

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// using llvm::PassManagerBuilder
// using llvm::RegisterStandardPasses

#include "llvm/Support/raw_ostream.h"
// using llvm::raw_ostream

#include "llvm/Support/Debug.h"
// using DEBUG macro
// using llvm::dbgs

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

template <typename T> struct make_plain_ptr { using type = T; };
template <typename T> struct make_plain_ptr<const T *> { using type = T *; };
template <typename T> using make_plain_ptr_t = typename make_plain_ptr<T>::type;

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

PropagateAttributesPass::PropagateAttributesPass()
    : llvm::CallGraphSCCPass(ID) {
  m_AttrBuilder.addAttribute(llvm::Attribute::NoReturn)
      .addAttribute("icsa.dynapar.io");

  return;
}

void PropagateAttributesPass::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.addRequired<llvm::CallGraphWrapperPass>();
  AU.setPreservesAll();

  return;
}

bool PropagateAttributesPass::doInitialization(llvm::CallGraph &CG) {
  for (const auto &CurNode : CG) {
    auto *CurFunc = CurNode.first;
    if (!CurFunc)
      continue;

    const auto &CurFuncAttrSet = CurFunc->getAttributes().getFnAttributes();
    llvm::AttrBuilder CurFuncAttrBuilder{CurFuncAttrSet, 0};

    if (CurFuncAttrBuilder.overlaps(m_AttrBuilder))
      m_PotentialCallers.insert(
          const_cast<make_plain_ptr_t<decltype(CurFunc)>>(CurFunc));
  }

  return false;
}

bool PropagateAttributesPass::runOnSCC(llvm::CallGraphSCC &SCC) {
  llvm::CallGraph &CG =
      getAnalysis<llvm::CallGraphWrapperPass>().getCallGraph();

  // selection phase

  for (auto &Node : SCC) {
    auto *Func = Node->getFunction();
    if (Func)
      m_PotentialCallers.insert(Func);

    PLUGIN_OUT << "ref #: " << Node->getNumReferences() << "\n";
  }

  return false;
}

} // namespace unnamed end
