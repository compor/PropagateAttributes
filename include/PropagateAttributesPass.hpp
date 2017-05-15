//
//
//

#ifndef PROPAGATEATTRIBUTESPASS_HPP
#define PROPAGATEATTRIBUTESPASS_HPP

#include "llvm/IR/Attributes.h"
// using llvm::Attribute
// using llvm::AttrBuilder

#include "llvm/Support/CommandLine.h"
// using llvm::cl::list

#include <string>
// using std::string

namespace llvm {
class ModulePass;
class AnalysisUsage;
class Module;
class StringRef;
} // namespace llvm end

namespace {

struct PropagateAttributesPass : public llvm::ModulePass {
  static char ID;

  PropagateAttributesPass();
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
  bool runOnModule(llvm::Module &M) override;

private:
  llvm::Attribute::AttrKind lookupTIAttribute(const llvm::StringRef &name);
  void checkCmdLineOptions(const llvm::cl::list<std::string> &options);

  static const std::map<llvm::StringRef, llvm::Attribute::AttrKind> TIAttrs;
};

} // namespace unnamed end

#endif // PROPAGATEATTRIBUTESPASS_HPP
