#include <iostream>

#include "ast.h"
#include "context.h"
#include "error/ast-exception.h"
#include "lexer.h"
#include "location.h"
#include "parser.h"
#include "pprinter.h"
#include "token.h"

using namespace std;

int main(int argc, char** argv) {
  string input = R"(
letrec plus:Nat->Nat->Nat =
  lambda a:Nat b:Nat.
    if iszero a
      then b
      else plus (pred a) (succ b);

letrec sum:List[Nat]->Nat =
  lambda l:List[Nat].
    if isnil l
      then 0
      else plus (head l) (sum (tail l));

let l = cons 1 (cons 2 nil[Nat]);

sum l;
)";

  std::unique_ptr<Lexer> lexer(Lexer::Create(input));
  Parser parser(lexer.get());
  vector<unique_ptr<Stmt>> stmts;
  Context ctx;
  PrettyPrinter pprinter(&ctx);

  try {
    stmts = parser.ParseAST(nullptr);
  } catch (const ast_exception& e) {
    lexer->locator()->Error(2, e.location(), e.what());
    return 0;
  }

  for (int i = 0; i < stmts.size(); ++i) {
    EvalStmt* eval_stmt = dynamic_cast<EvalStmt*>(stmts[i].get());
    BindTermStmt* term_stmt = dynamic_cast<BindTermStmt*>(stmts[i].get());
    BindTypeStmt* type_stmt = dynamic_cast<BindTypeStmt*>(stmts[i].get());

    if (eval_stmt != nullptr) {
      cout << pprinter.PrettyPrint(eval_stmt->term()) << endl;
    } else if (term_stmt != nullptr) {
      cout << pprinter.PrettyPrint(term_stmt->term()) << endl;
      ctx.AddName(term_stmt->variable());
    } else if (type_stmt != nullptr) {
      cout << pprinter.PrettyPrint(type_stmt->type()) << endl;
      ctx.AddName(type_stmt->type_alias());
    }
  }

  return 0;
}
