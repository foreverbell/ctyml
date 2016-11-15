#include <cstdio>

#include "common.h"
#include "token.h"

using namespace std;

int main() {
  printf("%d\n", TokenType::If);

  std::unique_ptr<Token> token;
  if (Token::CreateInt(Location(1, 1), 123, &token)) {
    printf("%d\n", token->number());
  }

  puts("Hello World!");
  return 0;
}
