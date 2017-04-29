//
//
//

#ifndef PROPAGATEATTRIBUTESPASS_HPP
#define PROPAGATEATTRIBUTESPASS_HPP

#include "llvm/Analysis/CallGraphSCCPass.h"
// using llvm::CallGraphSCCPass
// using llvm::CallGraphSCC

// TODO to move to source file
#include "llvm/Analysis/CallGraph.h"
#include "llvm/ADT/SmallPtrSet.h"

template <typename T> struct rm_const_ptr { using type = T; };
template <typename T> struct rm_const_ptr<const T *> { using type = T *; };
template <typename T> using rm_const_ptr_t = typename rm_const_ptr<T>::type;

#include <set>
// using std::set

#include <string>
// using std::string

namespace llvm {
class CallGraphSCCPass;
class CallGraphSCC;
class CallGraph;
class AnalysisUsage;
class AttrBuilder;
template <typename PtrType, unsigned> class SmallPtrSet;
class Function;
template <typename PtrType, unsigned> class SmallPtrSet;
} // namespace llvm end

using FuncSet = llvm::SmallPtrSet<llvm::Function *, 8>;
using ConstFuncSet = llvm::SmallPtrSet<const llvm::Function *, 8>;

namespace {

struct PropagateAttributesPass : public llvm::CallGraphSCCPass {
  static char ID;

  static ConstFuncSet filterFuncWithAttributes(const llvm::CallGraph &CG,
                                          const llvm::AttrBuilder &AB) {
    ConstFuncSet Funcs;

    for (const auto &CGNode : CG) {
      if (!CGNode.first)
        continue;

      Funcs.insert(CGNode.first);
    }

    return Funcs;
  }

  PropagateAttributesPass();
  bool doInitialization(llvm::CallGraph &CG) override;
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
  bool runOnSCC(llvm::CallGraphSCC &SCC) override;

private:
  llvm::AttrBuilder m_AttrBuilder;
  llvm::SmallPtrSet<llvm::Function *, 8> m_PotentialCallers;
  std::set<std::string> m_CustomAttributes;
};

} // namespace unnamed end

#endif // end of include guard: PROPAGATEATTRIBUTESPASS_HPP
