//
//
//

#include "llvm/Analysis/CallGraphSCCPass.h"
// using llvm::CallGraphSCCPass

namespace llvm {
class CallGraphSCCPass;
class CallGraphSCC;
} // namespace llvm end

namespace {

class PropagateAttributesPass : public llvm::CallGraphSCCPass {
public:
  static char ID;

  PropagateAttributesPass() : llvm::CallGraphSCCPass(ID) {}

  bool runOnSCC(llvm::CallGraphSCC &SCC) override;
};

} // namespace unnamed end
