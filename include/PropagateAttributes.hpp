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

namespace PropagateAttributes {

FuncSet filterFuncWithAttributes(const llvm::CallGraph &CG,
                                 const llvm::AttrBuilder &AB);

FuncSet getTransitiveCallers(const llvm::CallGraph &CG, const FuncSet &Callees);

bool propagate(const llvm::CallGraph &CG, const llvm::AttrBuilder &NewAB);

} // namespace PropagateAttributes end

#endif // PROPAGATEATTRIBUTES_HPP
