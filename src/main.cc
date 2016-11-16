#include <cstdio>

#include "ast.h"
#include "location.h"
#include "token.h"

using namespace std;

int main() {
  printf("%d\n", TokenType::If);

  Token* token;
  if ((token = Token::CreateInt(Location(1, 1), 123))) {
    printf("%d\n", token->number());
  }

  puts("Hello World!");
  return 0;
}
