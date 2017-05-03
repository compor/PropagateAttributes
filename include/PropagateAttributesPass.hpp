//
//
//

#ifndef PROPAGATEATTRIBUTESPASS_HPP
#define PROPAGATEATTRIBUTESPASS_HPP

// TODO to move to source file
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/CallGraphSCCPass.h"
#include "llvm/Support/raw_ostream.h"

template <typename T> struct rm_const_ptr { using type = T; };
template <typename T> struct rm_const_ptr<const T *> { using type = T *; };
template <typename T> using rm_const_ptr_t = typename rm_const_ptr<T>::type;

#include <set>
// using std::set

#include <string>
// using std::string

namespace llvm {
class ModulePass;
class AnalysisUsage;
class CallGraph;
class AttrBuilder;
class Module;
class Function;
template <typename PtrType, unsigned> class SmallPtrSet;
} // namespace llvm end

using FuncSet = llvm::SmallPtrSet<llvm::Function *, 8>;
// using ConstFuncSet = llvm::SmallPtrSet<const llvm::Function *, 8>;
using ConstFuncSet = std::set<const llvm::Function *>;

namespace {

struct PropagateAttributesPass : public llvm::ModulePass {
  static char ID;

  static ConstFuncSet filterFuncWithAttributes(const llvm::CallGraph &CG,
                                               const llvm::AttrBuilder &AB) {
    ConstFuncSet Funcs;

    for (const auto &CGNode : CG) {
      if (!CGNode.first)
        continue;

      auto *CurFunc = CGNode.first;
      llvm::AttrBuilder CurAB(CurFunc->getAttributes(),
                              llvm::AttributeSet::FunctionIndex);

      if (AB.overlaps(CurAB))
        Funcs.insert(CurFunc);
    }

    return Funcs;
  }

  using CGSCC_t = std::vector<llvm::CallGraphNode *>;

  static bool isCallerOf(const CGSCC_t &SCC,
                         const ConstFuncSet &PotentialCallees) {
    for (const auto &SCCNode : SCC) {
      const auto &found = std::find_if(
          std::begin(*SCCNode), std::end(*SCCNode), [&](const auto &e) {
            const auto &Callee = e.second->getFunction();
            return PotentialCallees.end() != PotentialCallees.find(Callee);
          });

      if (found != std::end(*SCCNode))
        return true;
    }

    return false;
  }

  static ConstFuncSet getTransitiveCallers(llvm::CallGraph &CG,
                                           const ConstFuncSet &Callees) {
    // initially add the callees in the set in order to find their immediate
    // callers, but remove them once done
    ConstFuncSet TransitiveCallers{Callees};

    llvm::scc_iterator<llvm::CallGraph *> SCCIter = llvm::scc_begin(&CG);
    decltype(SCCIter) SCCIterEnd = llvm::scc_end(&CG);

    for (; SCCIter != SCCIterEnd; ++SCCIter) {
      auto &CurSCC = *SCCIter;

      if (isCallerOf(CurSCC, TransitiveCallers))
        for (const auto &e : CurSCC)
          TransitiveCallers.insert(e->getFunction());

      // remove external node represented as null value
      TransitiveCallers.erase(nullptr);
    }

    for (const auto &e : Callees)
      TransitiveCallers.erase(e);

    return TransitiveCallers;
  }

  PropagateAttributesPass();
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
  bool runOnModule(llvm::Module &M) override;

private:
  llvm::Attribute::AttrKind lookupTIAttribute(const llvm::StringRef &name);
  void checkCmdLineOptions(const llvm::cl::list<std::string> &options);

  llvm::AttrBuilder m_AttrBuilder;
  llvm::SmallPtrSet<llvm::Function *, 8> m_PotentialCallers;
  std::set<std::string> m_CustomAttributes;

  static const std::map<llvm::StringRef, llvm::Attribute::AttrKind> TIAttrs;
};

} // namespace unnamed end

#endif // end of include guard: PROPAGATEATTRIBUTESPASS_HPP
