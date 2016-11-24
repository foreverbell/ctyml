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

using std::string;
using std::unique_ptr;
using std::vector;

class TypeCheckerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const string predefined_types = R"(
type N = Nat;
type B = Bool;
type T = N->N;
type F = {x: N, y: N};
)";

    unique_ptr<Lexer> lexer(Lexer::Create(predefined_types));
    assert(lexer != nullptr);
    Parser parser(lexer.get());

    predefined_type_stmts_ = parser.ParseAST(nullptr);
    assert(predefined_type_stmts_.size() == 4);
    for (size_t i = 0; i < predefined_type_stmts_.size(); ++i) {
      const BindTypeStmt* stmt = dynamic_cast<const BindTypeStmt*>(predefined_type_stmts_[i].get());
      ctx_.AddBinding(stmt->type_alias(), new Binding(nullptr, stmt->type()));
    }
  }

  unique_ptr<TermType> CreateType(const string& type) {
    unique_ptr<Lexer> lexer(Lexer::Create("type XXX = " + type + ";"));
    assert(lexer != nullptr);
    Parser parser(lexer.get());

    vector<unique_ptr<Stmt>> stmts = parser.ParseAST(&ctx_);
    ctx_.DropBindings(1);
    assert(ctx_.size() == 4);
    assert(stmts.size() == 1);
    const BindTypeStmt* stmt = dynamic_cast<const BindTypeStmt*>(stmts[0].get());
    return unique_ptr<TermType>(stmt->type()->clone());
  }

  bool CompareType(const string& lhs, const string& rhs) {
    return CreateType(lhs)->Compare(&ctx_, CreateType(rhs).get());
  }

  Context ctx_;
  vector<unique_ptr<Stmt>> predefined_type_stmts_;
};

TEST_F(TypeCheckerTest, TypeComparatorTest) {
  EXPECT_TRUE(CompareType("Nat", "N"));
  EXPECT_TRUE(CompareType("Bool", "B"));
  EXPECT_TRUE(CompareType("Nat->Nat", "T"));
  EXPECT_TRUE(CompareType("N->Nat", "T"));
  EXPECT_TRUE(CompareType("Nat->N", "T"));
  EXPECT_TRUE(CompareType("N->N", "T"));
  EXPECT_TRUE(CompareType("{x:Nat,y:Nat}", "F"));
  EXPECT_TRUE(CompareType("{x:N,y:Nat}", "F"));
  EXPECT_TRUE(CompareType("{x:Nat,y:N}", "F"));
  EXPECT_TRUE(CompareType("{x:N,y:N}", "F"));

  EXPECT_FALSE(CompareType("Bool", "N"));
  EXPECT_FALSE(CompareType("Nat", "B"));
  EXPECT_FALSE(CompareType("Nat->Bool", "T"));
  EXPECT_FALSE(CompareType("Nat", "T"));
  EXPECT_FALSE(CompareType("{x:Nat}", "F"));
  EXPECT_FALSE(CompareType("{x:Bool,y:Nat}", "F"));
  EXPECT_FALSE(CompareType("{x:Nat,y:B}", "F"));
}
