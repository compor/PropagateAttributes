//
//
//

#define DEBUG_TYPE "propagate_attributes"

#include "llvm/Pass.h"
// using llvm::RegisterPass

#include "llvm/IR/LegacyPassManager.h"
// using llvm::PassManagerBase

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/Analysis/CallGraph.h"
// using llvm::CallGraph

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// using llvm::PassManagerBuilder
// using llvm::RegisterStandardPasses

#include "llvm/ADT/SmallString.h"
// using llvm::SmallString

#include "llvm/ADT/StringRef.h"
// using llvm::StringRef

#include "llvm/Support/raw_ostream.h"
// using llvm::raw_fd_ostream

#include "llvm/Support/CommandLine.h"
// using llvm::cl::opt
// using llvm::cl::desc

#include "llvm/Support/FileSystem.h"
// using llvm::sys::fs::OpenFlags

#include "llvm/Support/Debug.h"
// using DEBUG macro
// using llvm::dbgs

#include <functional>
// using std::bind
// using std::placeholders

#include "PropagateAttributes.hpp"
#include "PropagateAttributesPass.hpp"

#ifndef NDEBUG
#define PLUGIN_OUT llvm::outs()
//#define PLUGIN_OUT llvm::nulls()

// convenience macro when building against a NDEBUG LLVM
#undef DEBUG
#define DEBUG(X)                                                               \
  do {                                                                         \
    X;                                                                         \
  } while (0);
#else // NDEBUG
#define PLUGIN_OUT llvm::dbgs()
#endif // NDEBUG

#define PLUGIN_ERR llvm::errs()

// plugin registration for opt

char PropagateAttributesPass::ID = 0;
static llvm::RegisterPass<PropagateAttributesPass>
    X("propagate-attributes", "propagate function attributes pass", false,
      false);

// plugin registration for clang

// the solution was at the bottom of the header file
// 'llvm/Transforms/IPO/PassManagerBuilder.h'
// create a static free-floating callback that uses the legacy pass manager to
// add an instance of this pass and a static instance of the
// RegisterStandardPasses class

static void
registerPropagateAttributesPass(const llvm::PassManagerBuilder &Builder,
                                llvm::legacy::PassManagerBase &PM) {
  PM.add(new PropagateAttributesPass());

  return;
}

static llvm::RegisterStandardPasses RegisterPropagateAttributesPass(
    llvm::PassManagerBuilder::EP_EarlyAsPossible,
    registerPropagateAttributesPass);

// command-line options

const std::map<llvm::StringRef, llvm::Attribute::AttrKind>
    PropagateAttributesPass::TIAttrs{{"noreturn", llvm::Attribute::NoReturn}};

static llvm::cl::opt<std::string> ReportStatsFilename(
    "pattr-stats",
    llvm::cl::desc("propagate attributes stats report filename"));

static llvm::cl::list<std::string> TDAttributesListOptions(
    "pattr-td-attr",
    llvm::cl::desc(
        "Specify target-dependent attributes to propagate up the call graph"),
    llvm::cl::CommaSeparated, llvm::cl::ZeroOrMore, llvm::cl::NotHidden);

static llvm::cl::list<std::string> TIAttributesListOptions(
    "pattr-ti-attr",
    llvm::cl::desc(
        "Specify target-independent attributes to propagate up the call graph"),
    llvm::cl::CommaSeparated, llvm::cl::ZeroOrMore, llvm::cl::NotHidden);

//

namespace {

class PropagateAttributesStats {
public:
  PropagateAttributesStats(llvm::StringRef FilenamePrefix)
      : m_FilenamePrefix(FilenamePrefix) {}
  void onEvent(llvm::Function *Func) { FuncsProcessed.insert(Func); }

  void clear() { FuncsProcessed.clear(); }

  void report(const llvm::StringRef FilenameSuffix) {
    std::error_code err;

    const auto &filename = m_FilenamePrefix + FilenameSuffix.str();
    llvm::raw_fd_ostream report(filename.getSingleStringRef(), err,
                                llvm::sys::fs::F_Text);

    if (err)
      PLUGIN_ERR << "could not open file: \"" << ReportStatsFilename
                 << "\" reason: " << err.message() << "\n";
    else {
      report << FuncsProcessed.size() << "\n";

      for (const auto &func : FuncsProcessed)
        if (func->hasName())
          report << func->getName() << "\n";
    }

    return;
  }

private:
  llvm::SmallString<16> m_FilenamePrefix;
  FuncSet FuncsProcessed;
};

PropagateAttributesPass::PropagateAttributesPass() : llvm::ModulePass(ID) {
  return;
}

void PropagateAttributesPass::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  AU.addRequired<llvm::CallGraphWrapperPass>();

  return;
}

bool PropagateAttributesPass::runOnModule(llvm::Module &M) {
  checkCmdLineOptions(TIAttributesListOptions);
  bool shouldReportStats = !ReportStatsFilename.empty();
  bool hasChanged = false;

  const auto &CG = getAnalysis<llvm::CallGraphWrapperPass>().getCallGraph();
  llvm::AttrBuilder tdAB;

  PropagateAttributesStats filteredStats("pattr-filtered-");
  PropagateAttributesStats propagatedStats("pattr-propagated-");

  PropagateAttributes propattr;
  if (shouldReportStats) {
    propattr.registerObserver(
        PropagateAttributes::EventType::FILTERED_FUNC_EVENT,
        std::bind(&PropagateAttributesStats::onEvent, &filteredStats,
                  std::placeholders::_1));

    propattr.registerObserver(
        PropagateAttributes::EventType::PROPAGATED_FUNC_EVENT,
        std::bind(&PropagateAttributesStats::onEvent, &propagatedStats,
                  std::placeholders::_1));
  }

  for (const auto &e : TDAttributesListOptions) {
    tdAB.addAttribute(e);
    hasChanged = propattr.propagate(CG, tdAB);
    tdAB.clear();

    if (shouldReportStats) {
      filteredStats.report(e);
      propagatedStats.report(e);
      filteredStats.clear();
      propagatedStats.clear();
    }
  }

  llvm::AttrBuilder tiAB;
  for (const auto &e : TIAttributesListOptions) {
    tiAB.addAttribute(lookupTIAttribute(e));
    hasChanged = propattr.propagate(CG, tiAB);
    tiAB.clear();

    if (shouldReportStats) {
      filteredStats.report(e);
      propagatedStats.report(e);
      filteredStats.clear();
      propagatedStats.clear();
    }
  }

  return hasChanged;
}

void PropagateAttributesPass::checkCmdLineOptions(
    const llvm::cl::list<std::string> &options) {
  for (const auto &e : options) {
    const auto &foundKind = lookupTIAttribute(e);

    if (llvm::Attribute::None == foundKind) {
      PLUGIN_ERR << "could not find option for target independent attribute: \'"
                 << e << "\'\n";

      PLUGIN_ERR << "supported attributes are: ";

      for (const auto &e : TIAttrs)
        PLUGIN_ERR << "\'" << e.first << "\' ";
      PLUGIN_ERR << "\n";

      std::exit(EXIT_FAILURE);
    }
  }

  return;
}

llvm::Attribute::AttrKind
PropagateAttributesPass::lookupTIAttribute(const llvm::StringRef &name) {
  const auto found = TIAttrs.find(name);
  return found == std::end(TIAttrs) ? llvm::Attribute::None : found->second;
}

} // namespace unnamed end
