#include "lexer.h"
#include "token.h"

#include <unordered_map>

using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;

namespace {

const unordered_map<string, TokenType> keyword_list = {
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

const unordered_map<char, TokenType> onechar_list = {
  {'.', TokenType::Dot},
  {',', TokenType::Comma},
  {':', TokenType::Colon},
  {';', TokenType::Semi},
  {'=', TokenType::Eq},
  {'_', TokenType::UScore},
  {'(', TokenType::LParen},
  {')', TokenType::RParen},
  {'{', TokenType::LCurly},
  {'}', TokenType::RCurly},
  {'[', TokenType::LBracket},
  {']', TokenType::RBracket},
};

size_t ParseNumber(const string& line, size_t line_number, size_t line_offset, unique_ptr<Token>* token) {
  assert(isdigit(line[line_offset]));

  int number = 0;
  size_t advance = 0;
  while (line_offset + advance < line.length() && isdigit(line[line_offset + advance])) {
    number = number * 10 + line[line_offset + advance] - '0';
    advance += 1;
  }
  if (!Token::CreateInt(Location(line_number, line_offset), number, token)) {
    return 0;
  }
  return advance;
}

size_t ParseIdentifer(const string& line, size_t line_number, size_t line_offset, unique_ptr<Token>* token) {
  assert(isalpha(line[line_offset]));

  size_t advance = 1;
  while (line_offset + advance < line.length()) {
    char ch = line[line_offset + advance];
    if (isdigit(ch) || isalpha(ch) || ch == '_' || ch == '\'') {
      ++advance;
    } else {
      break;
    }
  }

  const string identifier = line.substr(line_offset, advance);
  const Location location = Location(line_number, line_offset);
  const unordered_map<string, TokenType>::const_iterator iter = keyword_list.find(identifier);

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
size_t ParseToken(const string& line, size_t line_number, size_t line_offset, unique_ptr<Token>* token) {
#define create_token(token_type) \
  do { \
    if (!Token::Create(token_type, Location(line_number, line_offset), token)) { \
      return 0; \
    } \
  } while (false)

  // Handle single char token.
  const unordered_map<char, TokenType>::const_iterator iter = onechar_list.find(line[line_offset]);
  if (iter != onechar_list.end()) {
    create_token(iter->second);
    return 1;
  }

  // Handle multiple char token, natural number, identifier.
  if (line_offset + 1 < line.length() && line[line_offset] == '-' && line[line_offset + 1] == '>') {
    create_token(TokenType::Arrow);
    return 2;
  }

#undef create_token

  if (isdigit(line[line_offset])) {
    return ParseNumber(line, line_number, line_offset, token);
  }

  if (isalpha(line[line_offset])) {
    return ParseIdentifer(line, line_number, line_offset, token);
  }

  return 0;
}

}  // namespace

bool ScanTokens(const vector<string>& input, vector<unique_ptr<Token>>* tokens) {
  size_t line_number = 0;
  size_t line_offset = 0;
  unique_ptr<Token> token;

  for (const string& line : input) {
    ++line_number;
    line_offset = 0;
    while (line_offset < line.length()) {
      // Skip white spaces.
      if (line[line_offset] == ' ' || line[line_offset] == '\t') {
        ++line_offset;
        continue;
      }
      const int advance = ParseToken(line, line_number, line_offset, &token);
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
