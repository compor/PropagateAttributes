
#include <memory>
// using std::unique_ptr

#include <map>
// using std::map

#include <cassert>
// using assert

#include <cstdlib>
// using std::abort

#include "llvm/IR/LLVMContext.h"
// using llvm::LLVMContext

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/IR/LegacyPassManager.h"
// using llvm::legacy::PassMananger

#include "llvm/Pass.h"
// using llvm::Pass
// using llvm::PassInfo

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfoWrapperPass
// using llvm::LoopInfo

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

#include "PropagateAttributesPass.hpp"

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

    if(llvm::verifyModule(*m_Module, &(llvm::errs())))
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
        AU.setPreservesAll();
        //AU.addRequiredTransitive<llvm::CallGraphWrapperPass>();

        return;
      }

      bool runOnModule(llvm::Module &M) override {
        test_result_map::const_iterator found;
        const auto &CG = llvm::CallGraph(M);

        llvm::AttrBuilder AB;
        AB.addAttribute("foo");
        const auto &funcs =
            PropagateAttributesPass::filterFuncWithAttributes(CG, AB);

        // subcase
        found = lookup("functions found");

        const auto &ff = funcs.size();
        const auto &ev = boost::apply_visitor(test_result_visitor(), found->second);
        EXPECT_EQ(ev, ff) << found->first;

        return false;
      }

      test_result_map::const_iterator lookup(const std::string &subcase) {
        auto found = m_trm.find(subcase);
        if (m_trm.end() == found) {
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
  ParseAssemblyFile("test1.ll");

  test_result_map trm;

  trm.insert({"functions found", 0});
  ExpectTestPass(trm);
}

TEST_F(TestPropagateAttributes, DifferentAttribute) {
  ParseAssemblyFile("test02.ll");

  test_result_map trm;

  trm.insert({"functions found", 0});
  ExpectTestPass(trm);
}

} // namespace anonymous end
} // namespace icsa end
