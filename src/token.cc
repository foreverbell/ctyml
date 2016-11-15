#include "token.h"

using std::string;
using std::unique_ptr;

/* static */
bool Token::Create(TokenType type, Location location, unique_ptr<Token>* token) {
  if (type == TokenType::Int || type == TokenType::LCaseId || type == TokenType::UCaseId) {
    return false;
  }
  token->reset(new Token(type, location, -1, ""));
  return true;
}

/* static */
bool Token::CreateInt(Location location, int number, unique_ptr<Token>* token) {
  if (number < 0) {
    return false;
  }
  token->reset(new Token(TokenType::Int, location, number, ""));
  return true;
}

/* static */
bool Token::CreateId(Location location, const string& id, unique_ptr<Token>* token) {
  if (id.empty() || !isalpha(id[0])) {
    return false;
  }
  // Valid identifier should be led by an alphabet, followed by alphabets, digits, single-quotes or underscores.
  for (size_t i = 1; i < id.size(); ++i) {
    if (!isalpha(id[i]) && !isdigit(id[i]) && id[i] != '\'' && id[i] != '_') {
      return false;
    }
  }
  token->reset(new Token(id[0] >= 'A' && id[0] <= 'Z' ? TokenType::UCaseId : TokenType::LCaseId, location, -1, id));
  return true;
}
