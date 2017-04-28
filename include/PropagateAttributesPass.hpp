//
//
//

#ifndef PROPAGATEATTRIBUTESPASS_HPP
#define PROPAGATEATTRIBUTESPASS_HPP

#include "llvm/Analysis/CallGraphSCCPass.h"
// using llvm::CallGraphSCCPass
// using llvm::CallGraphSCC

namespace llvm {
class CallGraphSCCPass;
class CallGraphSCC;
class AnalysisUsage;
class AttrBuilder;
template <typename PtrType, unsigned> class SmallPtrSet;

} // namespace llvm end

namespace {

struct PropagateAttributesPass : public llvm::CallGraphSCCPass {
  static char ID;

  PropagateAttributesPass();
  bool doInitialization(llvm::CallGraph &CG) override;
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
  bool runOnSCC(llvm::CallGraphSCC &SCC) override;

private:
  llvm::AttrBuilder m_AttrBuilder;
  llvm::SmallPtrSet<llvm::Function *, 8> m_PotentialCallers;
};

} // namespace unnamed end

#endif // end of include guard: PROPAGATEATTRIBUTESPASS_HPP
