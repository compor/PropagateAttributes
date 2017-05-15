//
//
//

#include <memory>
// using std::unique_ptr

#include <map>
// using std::map

#include <algorithm>
// using std::for_each

#include <cassert>
// using assert

#include <cstdlib>
// using std::abort

#include "llvm/IR/LLVMContext.h"
// using llvm::getGlobalContext

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/Analysis/CallGraph.h"
// using llvm::CallGraphWrapperPass
// using llvm::CallGraph

#include "llvm/IR/LegacyPassManager.h"
// using llvm::legacy::PassMananger

#include "llvm/Pass.h"
// using llvm::Pass
// using llvm::PassInfo

#include "llvm/Support/SourceMgr.h"
// using llvm::SMDiagnostic

#include "llvm/AsmParser/Parser.h"
// using llvm::parseAssemblyString

#include "llvm/IR/Verifier.h"
// using llvm::verifyModule

#include "llvm/Support/ErrorHandling.h"
// using llvm::report_fatal_error

#include "llvm/Support/raw_ostream.h"
// using llvm::raw_string_ostream

#include "gtest/gtest.h"
// using testing::Test

#include "boost/variant.hpp"
// using boost::variant

#include "PropagateAttributes.hpp"

namespace icsa {
namespace {

using test_result_t = boost::variant<unsigned int>;
using test_result_map = std::map<std::string, test_result_t>;

struct test_result_visitor : public boost::static_visitor<unsigned int> {
  unsigned int operator()(unsigned int i) const { return i; }
};

class TestPropagateAttributes : public testing::Test {
public:
  TestPropagateAttributes()
      : m_Module{nullptr}, m_TestDataDir{"./unittests/data/"} {}

  void ParseAssemblyString(const char *Assembly) {
    llvm::SMDiagnostic err;

    m_Module =
        llvm::parseAssemblyString(Assembly, err, llvm::getGlobalContext());

    std::string errMsg;
    llvm::raw_string_ostream os(errMsg);
    err.print("", os);

    if (!m_Module)
      llvm::report_fatal_error(os.str().c_str());

    return;
  }

  void ParseAssemblyFile(const char *Filename) {
    llvm::SMDiagnostic err;

    std::string fullFilename{m_TestDataDir};
    fullFilename += Filename;

    m_Module =
        llvm::parseAssemblyFile(fullFilename, err, llvm::getGlobalContext());

    std::string errMsg;
    llvm::raw_string_ostream os(errMsg);
    err.print("", os);

    if (llvm::verifyModule(*m_Module, &(llvm::errs())))
      llvm::report_fatal_error("module verification failed\n");

    if (!m_Module)
      llvm::report_fatal_error(os.str().c_str());

    return;
  }

  void ExpectTestPass(const test_result_map &trm) {
    static char ID;

    struct UtilityPass : public llvm::ModulePass {
      UtilityPass(const test_result_map &trm)
          : llvm::ModulePass(ID), m_trm(trm) {}

      static int initialize() {
        auto *registry = llvm::PassRegistry::getPassRegistry();

        auto *PI = new llvm::PassInfo("Utility pass for unit tests", "", &ID,
                                      nullptr, true, true);

        registry->registerPass(*PI, false);
        llvm::initializeCallGraphWrapperPassPass(*registry);

        return 0;
      }

      void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
        AU.setPreservesCFG();
        AU.addRequired<llvm::CallGraphWrapperPass>();

        return;
      }

      bool runOnModule(llvm::Module &M) override {
        test_result_map::const_iterator found;
        auto &CG = getAnalysis<llvm::CallGraphWrapperPass>().getCallGraph();

        llvm::AttrBuilder AB;
        AB.addAttribute("foo");
        const auto &funcs =
            PropagateAttributes::filterFuncWithAttributes(CG, AB);

        FuncSet callees;

        std::for_each(std::begin(M), std::end(M), [&](auto &e) {
          if (e.getName().startswith("test"))
            callees.insert(&e);
        });

        const auto &callers =
            PropagateAttributes::getTransitiveCallers(CG, callees);

        // subcase
        found = lookup("functions found");
        if (found != std::end(m_trm)) {
          const auto &ev1 =
              boost::apply_visitor(test_result_visitor(), found->second);
          const auto &ff = funcs.size();

          EXPECT_EQ(ev1, ff) << found->first;
        }

        // subcase
        found = lookup("transitive function callers found");
        if (found != std::end(m_trm)) {
          const auto &ev2 =
              boost::apply_visitor(test_result_visitor(), found->second);
          const auto &fc = callers.size();

          EXPECT_EQ(ev2, fc) << found->first;
        }

        return false;
      }

      test_result_map::const_iterator lookup(const std::string &subcase,
                                             bool fatalIfMissing = false) {
        auto found = m_trm.find(subcase);
        if (fatalIfMissing && std::end(m_trm) == found) {
          llvm::errs() << "subcase: " << subcase << " test data not found\n";
          std::abort();
        }

        return found;
      }

      const test_result_map &m_trm;
    };

    static int init = UtilityPass::initialize();
    (void)init; // do not optimize away

    auto *P = new UtilityPass(trm);
    llvm::legacy::PassManager PM;

    PM.add(P);
    PM.run(*m_Module);

    return;
  }

protected:
  std::unique_ptr<llvm::Module> m_Module;
  const char *m_TestDataDir;
};

TEST_F(TestPropagateAttributes, NoAttributes) {
  ParseAssemblyFile("test01.ll");
  test_result_map trm;

  trm.insert({"functions found", 0});
  ExpectTestPass(trm);
}

TEST_F(TestPropagateAttributes, DoesNotHaveRequestedAttribute) {
  ParseAssemblyFile("test02.ll");
  test_result_map trm;

  trm.insert({"functions found", 0});
  ExpectTestPass(trm);
}

TEST_F(TestPropagateAttributes, HasRequestedAttribute) {
  ParseAssemblyFile("test03.ll");
  test_result_map trm;

  trm.insert({"functions found", 1});
  ExpectTestPass(trm);
}

TEST_F(TestPropagateAttributes, TransitiveCallersWithLeafSCC) {
  ParseAssemblyFile("test04.ll");
  test_result_map trm;

  trm.insert({"transitive function callers found", 2});
  ExpectTestPass(trm);
}

TEST_F(TestPropagateAttributes, TransitiveCallersWithNonLeafSCC) {
  ParseAssemblyFile("test05.ll");
  test_result_map trm;

  trm.insert({"transitive function callers found", 2});
  ExpectTestPass(trm);
}

TEST_F(TestPropagateAttributes, TransitiveCallersWithNonLeafSCCWithTwoEntries) {
  ParseAssemblyFile("test06.ll");
  test_result_map trm;

  trm.insert({"transitive function callers found", 5});
  ExpectTestPass(trm);
}

TEST_F(TestPropagateAttributes, TransitiveCallersWithTwoStartingSCCs) {
  ParseAssemblyFile("test07.ll");
  test_result_map trm;

  trm.insert({"transitive function callers found", 4});
  ExpectTestPass(trm);
}

TEST_F(TestPropagateAttributes,
       TransitiveCallersWithTwoStartingNonOverlappingSCCs) {
  ParseAssemblyFile("test08.ll");
  test_result_map trm;

  trm.insert({"transitive function callers found", 4});
  ExpectTestPass(trm);
}

} // namespace anonymous end
} // namespace icsa end
