#include "parser.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <sstream>

#include "ast.h"
#include "context.h"
#include "lexer.h"
#include "pprinter.h"

using std::string;
using std::stringstream;
using std::unique_ptr;
using std::vector;

class ParserTest : public ::testing::Test {
 public:
  ParserTest() : pprinter_(&ctx_) { }

 protected:
  void Init(const string& input) {
    lexer_.reset(Lexer::Create(input));
    ASSERT_TRUE(lexer_ != nullptr);
  }

  void TestThrows(const string& throws) {
    // TODO(foreverbell): Implement it.
  }

  void Test(const string& output) {
    parser_ = std::make_unique<Parser>(lexer_.get());

    vector<unique_ptr<Stmt>> stmts;
    ASSERT_NO_THROW(stmts = parser_->ParseAST(nullptr));

    stringstream ss(output);
    vector<string> pprints;

    while (true) {
      string tmp;
      if (std::getline(ss, tmp)) {
        if (tmp.empty()) continue;
        pprints.push_back(tmp);
      } else break;
    }

    ASSERT_EQ(pprints.size(), stmts.size());

    for (int i = 0; i < stmts.size(); ++i) {
      EvalStmt* eval_stmt = dynamic_cast<EvalStmt*>(stmts[i].get());
      BindTermStmt* term_stmt = dynamic_cast<BindTermStmt*>(stmts[i].get());
      BindTypeStmt* type_stmt = dynamic_cast<BindTypeStmt*>(stmts[i].get());

      if (eval_stmt != nullptr) {
        EXPECT_EQ(pprints[i], pprinter_.PrettyPrint(eval_stmt->term()));
      } else if (term_stmt != nullptr) {
        EXPECT_EQ(pprints[i], pprinter_.PrettyPrint(term_stmt->term()));
        ctx_.AddName(term_stmt->variable());
      } else if (type_stmt != nullptr) {
        EXPECT_EQ(pprints[i], pprinter_.PrettyPrint(type_stmt->type()));
        ctx_.AddName(type_stmt->type_alias());
      } else {
        FAIL() << "unknown stmt.";
      }
    }
  }

  Context ctx_;
  PrettyPrinter pprinter_;

  unique_ptr<Lexer> lexer_;
  unique_ptr<Parser> parser_;
};

TEST_F(ParserTest, TypeTest) {
  Init(R"(
type T1 = Nat->Nat;
type T2 = Bool->T1->(Bool->Bool)->{x: Nat, y: Bool}->Unit;
type T3 = T2->T1->T2->List[Nat];
)");
  Test(R"(
Nat->Nat
Bool->T1->(Bool->Bool)->{x:Nat,y:Bool}->Unit
T2->T1->T2->List[Nat]
)");
}

TEST_F(ParserTest, TermTest) {
  Init(R"(
let x = 10;
let y = false;
(if y then {x: true, y: unit} else {x: false, y: unit}).x;
)");
  Test(R"(
10
false
(if y then {x:true,y:unit} else {x:false,y:unit}).x
)");
}

TEST_F(ParserTest, LetRecTest) {
  Init(R"(
letrec equal:Nat->Nat->Bool =
  lambda a:Nat b:Nat.
    if iszero a
      then iszero b
      else if iszero b
             then false
             else equal (pred a) (pred b);
)");
  Test(R"(
fix (lambda equal:Nat->Nat->Bool. lambda a:Nat. lambda b:Nat. if iszero a then iszero b else if iszero b then false else equal (pred a) (pred b))
)");
}

TEST_F(ParserTest, AppTermTest) {
  Init(R"(
(succ {x: 1, y: 2}).x;
succ ({x: 1, y: 2}.x);
head (cons (lambda x:Nat->Nat. x) nil[Nat->Nat]) (lambda x:Nat->Nat. x) (lambda x:Nat->Nat. x);
)");
  Test(R"(
(succ {x:1,y:2}).x
succ {x:1,y:2}.x
head (cons (lambda x:Nat->Nat. x) nil[Nat->Nat]) (lambda x:Nat->Nat. x) (lambda x:Nat->Nat. x)
)");
}
