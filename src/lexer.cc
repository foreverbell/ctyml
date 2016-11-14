#include "lexer.h"
#include "token.h"

#include <map>

using std::map;
using std::string;
using std::unique_ptr;
using std::vector;

namespace {

const map<string, TokenType> keyword_list = {
  {"if", TokenType::If},
  {"then", TokenType::Then},
  {"else", TokenType::Else},
  {"true", TokenType::True},
  {"false", TokenType::False},
  {"pred", TokenType::Pred},
  {"succ", TokenType::Succ},
  {"iszero", TokenType::IsZero},
  {"nil", TokenType::Nil},
  {"cons", TokenType::Cons},
  {"isnil", TokenType::IsNil},
  {"head", TokenType::Head},
  {"tail", TokenType::Tail},
  {"unit", TokenType::Unit},
  {"lambda", TokenType::Lambda},
  {"let", TokenType::Let},
  {"in", TokenType::In},
  {"letrec", TokenType::Letrec},
  {"type", TokenType::TypeAlias},
  {"as", TokenType::As},
  {"Bool", TokenType::Bool},
  {"Nat", TokenType::Nat},
  {"List", TokenType::List},
  {"Unit", TokenType::UUnit},
};

int parse_number(const string& line, int line_number, int line_offset, unique_ptr<Token>* token) {
  assert(isdigit(line[line_offset]));

  int number = 0, advance = 0;
  while (line_offset + advance < line.length() && isdigit(line[line_offset + advance])) {
    number = number * 10 + line[line_offset + advance] - '0';
    advance += 1;
  }
  if (!Token::CreateInt(Location(line_number, line_offset), number, token)) {
    return 0;
  }
  return advance;
}

int parse_identifer(const string& line, int line_number, int line_offset, unique_ptr<Token>* token) {
  assert(isalpha(line[line_offset]));

  int advance = 1;
  while (line_offset + advance < line.length()) {
    char ch = line[line_offset + advance];
    if (isdigit(ch) || isalpha(ch) || ch == '_' || ch == '\'') {
      ++advance;
    } else {
      break;
    }
  }

  const string identifier = line.substr(line_offset, advance);
  Location location = Location(line_number, line_offset);
  map<string, TokenType>::const_iterator iter = keyword_list.find(identifier);

  if (iter == keyword_list.end()) {
    if (!Token::CreateId(location, identifier, token)) {
      return 0;
    }
  } else {
    if (!Token::Create(iter->second, location, token)) {
      return 0;
    }
  }
  return advance;
}

// Parses a single token, and returns the advance delta of line offset.
int parse_token(const string& line, int line_number, int line_offset, unique_ptr<Token>* token) {
#define create_token(token_type) \
  do { \
    if (!Token::Create(token_type, Location(line_number, line_offset), token)) { \
      return 0; \
    } \
  } while (false)

  // Handle single char token.
  switch (line[line_offset]) {
  case '.':
    create_token(TokenType::Dot);
    return 1;
  case ',':
    create_token(TokenType::Comma);
    return 1;
  case ':':
    create_token(TokenType::Colon);
    return 1;
  case ';':
    create_token(TokenType::Semi);
    return 1;
  case '=':
    create_token(TokenType::Eq);
    return 1;
  case '_':
    create_token(TokenType::UScore);
    return 1;
  case '(':
    create_token(TokenType::LParen);
    return 1;
  case ')':
    create_token(TokenType::RParen);
    return 1;
  case '{':
    create_token(TokenType::LCurly);
    return 1;
  case '}':
    create_token(TokenType::RCurly);
    return 1;
  case '[':
    create_token(TokenType::LBracket);
    return 1;
  case ']':
    create_token(TokenType::RBracket);
    return 1;
  }

  // Handle multiple char token, natural number, identifier.
  if (line_offset + 1 < line.length() && line[line_offset] == '-' && line[line_offset + 1] == '>') {
    create_token(TokenType::Arrow);
    return 2;
  }

  if (isdigit(line[line_offset])) {
    return parse_number(line, line_number, line_offset, token);
  }

  if (isalpha(line[line_offset])) {
    return parse_identifer(line, line_number, line_offset, token);
  }

#undef create_token

  return 0;
}

}  // namespace


bool ScanTokens(const vector<string>& input, vector<unique_ptr<Token>>* tokens) {
  tokens->clear();

  int line_number = 0, line_offset = 0;
  for (const string& line : input) {
    ++line_number;
    line_offset = 0;
    while (line_offset < line.length()) {
      // Skip white spaces.
      if (line[line_offset] == ' ' || line[line_offset] == '\t') {
        ++line_offset;
        continue;
      }
      unique_ptr<Token> token;
      const int advance = parse_token(line, line_number, line_offset, &token);
      if (advance == 0) {
        return false;
      } else {
        tokens->push_back(std::move(token));
        line_offset += advance;
      }
    }
  }

  return true;
}
