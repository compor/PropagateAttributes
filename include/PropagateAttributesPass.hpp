//
//
//

#include "llvm/Pass.h"
// using llvm::CallGraphSCCPass

namespace llvm {
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
