//
//
//

#include "llvm/Analysis/CallGraphSCCPass.h"
// using llvm::CallGraphSCCPass

namespace llvm {
class CallGraphSCCPass;
class CallGraphSCC;
class AnalysisUsage;
} // namespace llvm end

namespace {

class PropagateAttributesPass : public llvm::CallGraphSCCPass {
public:
  static char ID;

  PropagateAttributesPass() : llvm::CallGraphSCCPass(ID) {}

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
  bool runOnSCC(llvm::CallGraphSCC &SCC) override;
};

} // namespace unnamed end
