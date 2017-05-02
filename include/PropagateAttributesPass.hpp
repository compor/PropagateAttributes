//
//
//

#ifndef PROPAGATEATTRIBUTESPASS_HPP
#define PROPAGATEATTRIBUTESPASS_HPP

// TODO to move to source file
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SCCIterator.h"

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

  static ConstFuncSet getSCCCallers(const llvm::CallGraph &CG,
                                    const ConstFuncSet &Callees) {
    ConstFuncSet SCCFuncs;
    ConstFuncSet SCCCallers;

    //for (const auto &CGNode : CGSCC)
      //SCCFuncs.insert(CGNode->getFunction());

    //std::for_each(std::begin(SCCFuncs), std::end(SCCFuncs), [&](const auto &e) {
      //if (Callees.end() != Callees.find(e))
        //SCCCallers.insert(e);
      //llvm::outs() << e->getName() << "\n";
    //});

    // std::find_if(std::begin(CGSCC), std::end(CGSCC), [&](const auto &e) {
    // for (const auto &callee : Callees) {
    // if (callee == e->getFunction())
    // llvm::outs() << "---" << *callee;
    //}

    // return true;
    //});

    return SCCCallers;
  }

  PropagateAttributesPass();
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
  bool runOnModule(llvm::Module &M) override;

private:
  llvm::AttrBuilder m_AttrBuilder;
  llvm::SmallPtrSet<llvm::Function *, 8> m_PotentialCallers;
  std::set<std::string> m_CustomAttributes;
};

} // namespace unnamed end

#endif // end of include guard: PROPAGATEATTRIBUTESPASS_HPP
