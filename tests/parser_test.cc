#include "parser.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "ast.h"
#include "context.h"
#include "lexer.h"
#include "pprinter.h"
#include "test-utils.h"

using std::string;
using std::unique_ptr;
using std::vector;

class ParserTest : public ::testing::Test {
 protected:
  void TestParserThrows(const string& input, const string& throws) {
    // TODO(foreverbell): Implement it.
  }

  void TestParser(const string& input, const string& output) {
    unique_ptr<Lexer> lexer(Lexer::Create(input));
    unique_ptr<Parser> parser = std::make_unique<Parser>(lexer.get());
    Context ctx;
    PrettyPrinter pprinter(&ctx);

    vector<unique_ptr<Stmt>> stmts;
    ASSERT_NO_THROW(stmts = parser->ParseAST(nullptr));
    vector<string> pprints = SplitByLine(output);

    ASSERT_EQ(pprints.size(), stmts.size());

    for (size_t i = 0; i < stmts.size(); ++i) {
      EvalStmt* eval_stmt = dynamic_cast<EvalStmt*>(stmts[i].get());
      BindTermStmt* term_stmt = dynamic_cast<BindTermStmt*>(stmts[i].get());
      BindTypeStmt* type_stmt = dynamic_cast<BindTypeStmt*>(stmts[i].get());

      if (eval_stmt != nullptr) {
        EXPECT_EQ(pprints[i], pprinter.PrettyPrint(eval_stmt->term().get()));
      } else if (term_stmt != nullptr) {
        EXPECT_EQ(pprints[i], pprinter.PrettyPrint(term_stmt->term().get()));
        ctx.AddName(term_stmt->variable());
      } else if (type_stmt != nullptr) {
        EXPECT_EQ(pprints[i], pprinter.PrettyPrint(type_stmt->type().get()));
        ctx.AddName(type_stmt->type_alias());
      } else {
        FAIL() << "unknown stmt.";
      }
    }
  }
};

TEST_F(ParserTest, TypeTest) {
  TestParser(R"(
type T1 = Nat->Nat;
type T2 = Bool->T1->(Bool->Bool)->{x: Nat, y: Bool}->Unit;
type T3 = T2->T1->T2->List[Nat];
10 as Nat;
lambda x:Nat. x as T1;
)", R"(
Nat->Nat
Bool->T1->(Bool->Bool)->{x:Nat,y:Bool}->Unit
T2->T1->T2->List[Nat]
(10) as Nat
lambda x:Nat. x as T1
)");
}

TEST_F(ParserTest, LetTest) {
  TestParser(R"(
let x = 10;
let x = 10 in x;
let y = false;
let y = false in y;
let _ = true in y;
(if y then {x: true, y: unit} else {x: false, y: unit}).x;
let z = y;
)", R"(
10
let x_1 = 10 in x_1
false
let y_1 = false in y_1
let _ = true in y
(if y then {x:true,y:unit} else {x:false,y:unit}).x
y
)");
}

TEST_F(ParserTest, LetRecTest) {
  TestParser(R"(
letrec _:Nat = 1 in 2;
letrec equal:Nat->Nat->Bool =
  lambda a:Nat b:Nat.
    if iszero a
      then iszero b
      else
        if iszero b
          then false
          else equal (pred a) (pred b);
letrec equal_list:List[Nat]->List[Nat]->Bool =
  lambda a:List[Nat] b:List[Nat].
    if isnil a
      then isnil b
      else
        if isnil b
          then false
          else
            if equal (head a) (head b)
              then equal_list (tail a) (tail b)
              else false
in equal_list (cons 3 nil[Nat]) (cons 3 (cons 2 nil[Nat]));
)", R"(
let _ = fix (lambda _:Nat. 1) in 2
fix (lambda equal:Nat->Nat->Bool. lambda a:Nat. lambda b:Nat. if iszero a then iszero b else if iszero b then false else equal (pred a) (pred b))
let equal_list = fix (lambda equal_list:List[Nat]->List[Nat]->Bool. lambda a:List[Nat]. lambda b:List[Nat]. if isnil a then isnil b else if isnil b then false else if equal (head a) (head b) then equal_list (tail a) (tail b) else false) in equal_list (cons (3) nil[Nat]) (cons (3) (cons (2) nil[Nat]))
)");
}

TEST_F(ParserTest, AppTermTest) {
  TestParser(R"(
(succ {x: 1, y: 2}).x;
succ ({x: 1, y: 2}.x);
head (cons (lambda x:Nat->Nat. x) nil[Nat->Nat]) (lambda x:Nat->Nat. x) (lambda x:Nat->Nat. x);
)", R"(
(succ {x:1,y:2}).x
succ {x:1,y:2}.x
head (cons (lambda x:Nat->Nat. x) nil[Nat->Nat]) (lambda x:Nat->Nat. x) (lambda x:Nat->Nat. x)
)");
}
