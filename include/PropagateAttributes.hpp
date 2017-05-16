//
//
//

#ifndef PROPAGATEATTRIBUTES_HPP
#define PROPAGATEATTRIBUTES_HPP

#include <set>
// using std::set

namespace llvm {
class ModulePass;
class AnalysisUsage;
class CallGraph;
class AttrBuilder;
class Module;
class Function;
} // namespace llvm end

template <typename T> struct rm_const_ptr { using type = T; };
template <typename T> struct rm_const_ptr<const T *> { using type = T *; };
template <typename T> using rm_const_ptr_t = typename rm_const_ptr<T>::type;

using FuncSet = std::set<llvm::Function *>;

class PropagateAttributes {
public:
  FuncSet filterFuncWithAttributes(const llvm::CallGraph &CG,
                                   const llvm::AttrBuilder &AB);
  FuncSet getTransitiveCallers(const llvm::CallGraph &CG,
                               const FuncSet &Callees);
  bool propagate(const llvm::CallGraph &CG, const llvm::AttrBuilder &NewAB);

protected:
  using CGSCC_t = std::vector<const llvm::CallGraphNode *>;

  bool isCallerOf(const CGSCC_t &SCC, const FuncSet &PotentialCallees);
};

#endif // PROPAGATEATTRIBUTES_HPP
