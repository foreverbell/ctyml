#include <cstdio>

#include "ast.h"
#include "lexer.h"
#include "location.h"
#include "parser.h"
#include "pprinter.h"
#include "token.h"

using namespace std;

int main() {
  printf("Parsing type T = Nat->(Nat->Naat)->Bool;\n");
  std::unique_ptr<Lexer> lexer(Lexer::Create("type T = Nat->(Nat->Naat)->Bool;"));
  Parser parser(lexer.get());
  auto stmts = parser.ParseAST();
  assert(stmts.size() == 1);
  BindTypeStmt* stmt = dynamic_cast<BindTypeStmt*>(stmts[0].get());
  assert(stmt != nullptr);

  PrettyPrinter pprinter;
  printf("%s\n", pprinter.PrettyPrint(stmt->type()).c_str());

  puts("Hello World!");
  return 0;
}
