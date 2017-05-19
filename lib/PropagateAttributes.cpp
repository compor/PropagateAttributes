//
//
//

#include "llvm/Analysis/CallGraph.h"
// using llvm::CallGraph
// using llvm::CallGraphNode

#include "llvm/ADT/SCCIterator.h"
// using llvm::scc_begin
// using llvm::scc_end

#include "llvm/IR/Attributes.h"
// using llvm::Attribute
// using llvm::AttrBuilder

#include <vector>
// using std::vector

#include "PropagateAttributes.hpp"

FuncSet
PropagateAttributes::filterFuncWithAttributes(const llvm::CallGraph &CG,
                                              const llvm::AttrBuilder &AB) {
  FuncSet Funcs;

  for (auto &CGNode : CG) {
    if (!CGNode.first)
      continue;

    auto *CurFunc = CGNode.first;
    llvm::AttrBuilder CurAB(CurFunc->getAttributes(),
                            llvm::AttributeSet::FunctionIndex);

    if (AB.overlaps(CurAB)) {
      auto tmpCurFunc = rm_const_ptr_t<decltype(CurFunc)>(CurFunc);
      Funcs.insert(tmpCurFunc);
      notify(EventType::FILTERED_FUNC_EVENT, tmpCurFunc);
    }
  }

  return Funcs;
}

FuncSet PropagateAttributes::getTransitiveCallers(const llvm::CallGraph &CG,
                                                  const FuncSet &Callees) {
  // initially add the callees in the set in order to find their immediate
  // callers, but remove them once done
  FuncSet TransitiveCallers{Callees};

  using CG_t = std::remove_reference_t<decltype(CG)>;

  llvm::scc_iterator<CG_t *> SCCIter = llvm::scc_begin(&CG);
  decltype(SCCIter) SCCIterEnd = llvm::scc_end(&CG);

  for (; SCCIter != SCCIterEnd; ++SCCIter) {
    auto &CurSCC = *SCCIter;

    if (isCallerOf(CurSCC, TransitiveCallers))
      for (const auto &e : CurSCC)
        TransitiveCallers.insert(e->getFunction());

    // remove external node represented as null value
    TransitiveCallers.erase(nullptr);
  }

  for (const auto &e : Callees)
    TransitiveCallers.erase(e);

  return TransitiveCallers;
}

bool PropagateAttributes::propagate(const llvm::CallGraph &CG,
                                    const llvm::AttrBuilder &NewAB) {
  bool hasChanged = false;
  auto curIndex = llvm::AttributeSet::FunctionIndex;

  const auto &funcs = filterFuncWithAttributes(CG, NewAB);
  const auto &callers = getTransitiveCallers(CG, funcs);

  for (auto &caller : callers) {
    auto &curCtx = caller->getContext();
    const auto &callerAS = caller->getAttributes();
    const auto &newAS = llvm::AttributeSet::get(curCtx, curIndex, NewAB);

    if (callerAS != newAS) {
      llvm::AttrBuilder CallerAB{callerAS, curIndex};
      CallerAB.merge(NewAB);

      caller->addAttributes(
          curIndex, llvm::AttributeSet::get(curCtx, curIndex, CallerAB));

      hasChanged = true;
    }
  }

  return hasChanged;
}

void PropagateAttributes::registerObserver(EventType e,
                                           EventCallback callback) {
  auto found = m_Subscribers.find(e);

  if (std::end(m_Subscribers) == found)
    m_Subscribers.insert({e, {callback}});
  else
    (found->second).push_back(callback);

  return;
}
// protected members

bool PropagateAttributes::isCallerOf(const CGSCC_t &SCC,
                                     const FuncSet &PotentialCallees) {
  for (const auto &SCCNode : SCC) {
    const auto &found = std::find_if(
        std::begin(*SCCNode), std::end(*SCCNode), [&](const auto &e) {
          const auto &Callee = e.second->getFunction();
          return PotentialCallees.end() != PotentialCallees.find(Callee);
        });

    if (found != std::end(*SCCNode))
      return true;
  }

  return false;
}

void PropagateAttributes::notify(EventType E, llvm::Function *Func) const {
  auto found = m_Subscribers.find(E);

  if (std::end(m_Subscribers) != found)
    for (const auto &callback : found->second)
      callback(Func);

  return;
}
