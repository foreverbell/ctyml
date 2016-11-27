#include "evaluator.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

#include "ast.h"
#include "context.h"
#include "error.h"
#include "evaluator.h"
#include "lexer.h"
#include "parser.h"
#include "pprinter.h"
#include "test-utils.h"
#include "type-checker.h"

using std::string;
using std::unique_ptr;
using std::vector;

class EvaluatorTest : public ::testing::Test {
 protected:
  void TestEvaluator(const string& input, const string& output) {
    unique_ptr<Lexer> lexer(Lexer::Create(input));
    unique_ptr<Parser> parser = std::make_unique<Parser>(lexer.get());
    PrettyPrinter pprinter(&ctx_);
    TypeChecker type_checker(&ctx_);
    TermEvaluator evaluator(&ctx_);

    vector<unique_ptr<Stmt>> stmts;
    ASSERT_NO_THROW(stmts = parser->ParseAST(&ctx_));
    vector<string> pprints = SplitByLine(output);

    ASSERT_EQ(pprints.size(), stmts.size());

    for (size_t i = 0; i < stmts.size(); ++i) {
      EvalStmt* eval_stmt = dynamic_cast<EvalStmt*>(stmts[i].get());
      BindTermStmt* term_stmt = dynamic_cast<BindTermStmt*>(stmts[i].get());

      if (eval_stmt != nullptr) {
        unique_ptr<TermType> type = type_checker.TypeCheck(eval_stmt->term().get());
        unique_ptr<Term> term;

        try {
          term = evaluator.Evaluate(eval_stmt->term().get());
        } catch (const runtime_exception& e) {
          EXPECT_EQ(pprints[i], e.what());
          continue;
        }
        EXPECT_EQ(pprints[i], pprinter.PrettyPrint(term.get()));
      } else if (term_stmt != nullptr) {
        unique_ptr<TermType> type = type_checker.TypeCheck(term_stmt->term().get());
        unique_ptr<Term> term;

        try {
          term = evaluator.Evaluate(term_stmt->term().get());
        } catch (const runtime_exception& e) {
          EXPECT_EQ(pprints[i], e.what());
          continue;
        }
        EXPECT_EQ(pprints[i], pprinter.PrettyPrint(term.get()));
        ctx_.AddBinding(term_stmt->variable(), new Binding(term.get(), type.get()));
        defined_types_.push_back(std::move(type));
        evaluated_terms_.push_back(std::move(term));
      } else {
        // Leave BindTypeStmt unhandled.
        FAIL() << "unknown stmt.";
      }
    }
  }

  Context ctx_;
  vector<unique_ptr<TermType>> defined_types_;
  vector<unique_ptr<Term>> evaluated_terms_;
};

TEST_F(EvaluatorTest, EmptyList) {
  TestEvaluator(R"(
let l = nil[Nat];
let l' = cons 1 l;
head l;
tail (tail l');
)", R"(
nil[Nat]
cons (1) nil[Nat]
runtime error: <head> on an empty list
runtime error: <tail> on an empty list
)");
}

TEST_F(EvaluatorTest, Pred0) {
  TestEvaluator(R"(
let x = pred 0;
let y = x;
y;
)", R"(
0
0
0
)");
}

TEST_F(EvaluatorTest, Field) {
  TestEvaluator(R"(
(if false then {x: 12} else {x: 23}).x;
(lambda b:Bool. (let bb = b in if (bb as Bool) then {x: 12} else {x: 23}).x) true;
)", R"(
23
12
)");

}

TEST_F(EvaluatorTest, ListSum) {
  TestEvaluator(R"(
letrec gen:Nat->List[Nat] =
  lambda x:Nat.
    if iszero x
      then nil[Nat]
      else cons x (gen (pred x));

let l_0 = gen 0;
let l_2 = (gen (2 as Nat)) as List[Nat];

isnil l_0;
(isnil l_2) as Bool;

letrec plus:Nat->Nat->Nat =
  lambda a:Nat b:Nat.
    if iszero a
      then b
      else plus (pred a) (succ b);

letrec sum:List[Nat]->Nat =
  lambda l:List[Nat].
    if isnil l
      then 0
      else plus (head l) (sum (tail l)) in sum (gen 23);
)", R"(
lambda x:Nat. if iszero x then nil[Nat] else cons x (fix (lambda gen:Nat->List[Nat]. lambda x_1:Nat. if iszero x_1 then nil[Nat] else cons x_1 (gen (pred x_1))) (pred x))
nil[Nat]
cons (2) (cons (1) nil[Nat])
true
false
lambda a:Nat. lambda b:Nat. if iszero a then b else fix (lambda plus:Nat->Nat->Nat. lambda a_1:Nat. lambda b_1:Nat. if iszero a_1 then b_1 else plus (pred a_1) (succ b_1)) (pred a) (succ b)
276
)");
}
