#include "type-checker.h"

#include <cassert>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "ast.h"
#include "context.h"
#include "lexer.h"
#include "parser.h"
#include "pprinter.h"
#include "test-utils.h"
#include "type-checker.h"
#include "type-helper.h"

using std::string;
using std::stringstream;
using std::unique_ptr;
using std::vector;

class TypeCheckerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const string predefined_types = R"(
type N = Nat;
type B = Bool;
type U = Unit;
type T = N->N;
type T1 = T;
type F = {x: N, y: N};
type L = List[T1];
)";

    unique_ptr<Lexer> lexer(Lexer::Create(predefined_types));
    assert(lexer != nullptr);
    Parser parser(lexer.get());
    vector<unique_ptr<Stmt>> stmts = parser.ParseAST(nullptr);

    assert(stmts.size() == 7);
    for (size_t i = 0; i < stmts.size(); ++i) {
      BindTypeStmt* stmt = dynamic_cast<BindTypeStmt*>(stmts[i].get());
      ctx_.AddBinding(stmt->type_alias(), new Binding(nullptr, stmt->type().get()));
      defined_types_.push_back(std::move(stmt->type()));
    }
  }

  Context* ctx() { return &ctx_; }

  unique_ptr<TermType> CreateType(const string& type) {
    unique_ptr<Lexer> lexer(Lexer::Create("type XXX = " + type + ";"));
    assert(lexer != nullptr);
    Parser parser(lexer.get());

    vector<unique_ptr<Stmt>> stmts = parser.ParseAST(&ctx_);
    assert(ctx_.size() == 7);
    assert(stmts.size() == 1);
    const BindTypeStmt* stmt = dynamic_cast<const BindTypeStmt*>(stmts[0].get());
    return unique_ptr<TermType>(stmt->type()->clone());
  }

  bool CompareType(const string& lhs, const string& rhs) {
    return CreateType(lhs)->Compare(&ctx_, CreateType(rhs).get());
  }

  unique_ptr<TermType> SimplifyType(const TermType* type) {
    return ::SimplifyType(&ctx_, type);
  }

  void TestTypeChecker(const string& input, const string& output) {
    unique_ptr<Lexer> lexer(Lexer::Create(input));
    unique_ptr<Parser> parser = std::make_unique<Parser>(lexer.get());
    PrettyPrinter pprinter(&ctx_);
    TypeChecker type_checker(&ctx_);

    vector<unique_ptr<Stmt>> stmts;
    ASSERT_NO_THROW(stmts = parser->ParseAST(&ctx_));
    vector<string> pprints = SplitByLine(output);

    ASSERT_EQ(pprints.size(), stmts.size());

    for (size_t i = 0; i < stmts.size(); ++i) {
      EvalStmt* eval_stmt = dynamic_cast<EvalStmt*>(stmts[i].get());
      BindTermStmt* term_stmt = dynamic_cast<BindTermStmt*>(stmts[i].get());
      BindTypeStmt* type_stmt = dynamic_cast<BindTypeStmt*>(stmts[i].get());

      if (eval_stmt != nullptr) {
        EXPECT_EQ(pprints[i], pprinter.PrettyPrint(type_checker.TypeCheck(eval_stmt->term().get()).get()));
      } else if (term_stmt != nullptr) {
        unique_ptr<TermType> type = type_checker.TypeCheck(term_stmt->term().get());
        EXPECT_EQ(pprints[i], term_stmt->variable() + " : " + 
                              pprinter.PrettyPrint(type_checker.TypeCheck(term_stmt->term().get()).get()));
        ctx_.AddBinding(term_stmt->variable(), new Binding(term_stmt->term().get(), type.get()));
        defined_types_.push_back(std::move(type));
      } else if (type_stmt != nullptr) {
        EXPECT_EQ(pprints[i], type_stmt->type_alias() + " = " + pprinter.PrettyPrint(type_stmt->type().get()));
        ctx_.AddBinding(type_stmt->type_alias(), new Binding(nullptr, type_stmt->type().get()));
        defined_types_.push_back(std::move(type_stmt->type()));
      } else {
        FAIL() << "unknown stmt.";
      }
    }
  }

 private:
  Context ctx_;
  vector<unique_ptr<TermType>> defined_types_;
};

TEST_F(TypeCheckerTest, SimplifyTypeTest) {
  PrettyPrinter pprinter(ctx());

  EXPECT_EQ(pprinter.PrettyPrint(SimplifyType(CreateType("N").get()).get()), "Nat");
  EXPECT_EQ(pprinter.PrettyPrint(SimplifyType(CreateType("B").get()).get()), "Bool");
  EXPECT_EQ(pprinter.PrettyPrint(SimplifyType(CreateType("U").get()).get()), "Unit");
  EXPECT_EQ(pprinter.PrettyPrint(SimplifyType(CreateType("T").get()).get()), "N->N");
  EXPECT_EQ(pprinter.PrettyPrint(SimplifyType(CreateType("T1").get()).get()), "N->N");
  EXPECT_EQ(pprinter.PrettyPrint(SimplifyType(CreateType("F").get()).get()), "{x:N,y:N}");
  EXPECT_EQ(pprinter.PrettyPrint(SimplifyType(CreateType("L").get()).get()), "List[T1]");
}

TEST_F(TypeCheckerTest, TypeComparatorTest) {
  EXPECT_TRUE(CompareType("Nat", "N"));
  EXPECT_TRUE(CompareType("Bool", "B"));
  EXPECT_TRUE(CompareType("U", "Unit"));
  EXPECT_TRUE(CompareType("Nat->Nat", "T"));
  EXPECT_TRUE(CompareType("N->Nat", "T"));
  EXPECT_TRUE(CompareType("Nat->N", "T"));
  EXPECT_TRUE(CompareType("N->N", "T"));
  EXPECT_TRUE(CompareType("{x:Nat,y:Nat}", "F"));
  EXPECT_TRUE(CompareType("{x:N,y:Nat}", "F"));
  EXPECT_TRUE(CompareType("{x:Nat,y:N}", "F"));
  EXPECT_TRUE(CompareType("{x:N,y:N}", "F"));
  EXPECT_TRUE(CompareType("List[T1]", "L"));
  EXPECT_TRUE(CompareType("List[T]", "L"));
  EXPECT_TRUE(CompareType("List[T]", "List[T1]"));

  EXPECT_FALSE(CompareType("Bool", "N"));
  EXPECT_FALSE(CompareType("Nat", "B"));
  EXPECT_FALSE(CompareType("Unit", "N"));
  EXPECT_FALSE(CompareType("Nat->Bool", "T"));
  EXPECT_FALSE(CompareType("Nat", "T"));
  EXPECT_FALSE(CompareType("{x:Nat}", "F"));
  EXPECT_FALSE(CompareType("{x:Bool,y:Nat}", "F"));
  EXPECT_FALSE(CompareType("{x:Nat,y:B}", "F"));
  EXPECT_FALSE(CompareType("List[N]", "L"));
}
