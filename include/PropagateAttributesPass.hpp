//
//
//

#include "llvm/Analysis/CallGraphSCCPass.h"
// using llvm::CallGraphSCCPass

namespace llvm {
class CallGraphSCCPass;
class CallGraphSCC;
class AnalysisUsage;
class AttrBuilder;
} // namespace llvm end

namespace {

struct PropagateAttributesPass : public llvm::CallGraphSCCPass {
  static char ID;

  PropagateAttributesPass();
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
  bool runOnSCC(llvm::CallGraphSCC &SCC) override;

private:
  llvm::AttrBuilder ABuilder;
};

} // namespace unnamed end
