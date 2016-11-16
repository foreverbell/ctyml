#include <cstdio>

#include "ast.h"
#include "location.h"
#include "token.h"
#include "lexer.h"
#include "parser.h"

using namespace std;

int main() {
  printf("%d\n", TokenType::If);

  Token* token;
  if ((token = Token::CreateInt(Location(1, 1), 123))) {
    printf("%d\n", token->number());
  } else;

  Lexer* lexer = Lexer::Create("if false then 42 then 23;");
  Parser parser(lexer);
  parser.lexer();

  puts("Hello World!");
  return 0;
}
