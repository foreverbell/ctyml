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

bool is_whitespaces(char ch) {
  return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

}  // namespace

size_t Lexer::ParseNumber(size_t offset, unique_ptr<Token>* token) {
  assert(isdigit(input_[offset]));

  int number = 0;
  size_t advance = 0;
  while (offset + advance < input_.length() && isdigit(input_[offset + advance])) {
    number = number * 10 + input_[offset + advance] - '0';
    advance += 1;
  }

  token->reset(Token::CreateInt(Location(offset, offset + advance), number));
  if (token == nullptr) {
    return 0;
  }
  return advance;
}

size_t Lexer::ParseIdentifer(size_t offset, unique_ptr<Token>* token) {
  assert(isalpha(input_[offset]));

  size_t advance = 1;
  while (offset + advance < input_.length()) {
    char ch = input_[offset + advance];
    if (isdigit(ch) || isalpha(ch) || ch == '_' || ch == '\'') {
      ++advance;
    } else {
      break;
    }
  }

  const string identifier = input_.substr(offset, advance);
  const Location location = Location(offset, offset + advance);
  const unordered_map<string, TokenType>::const_iterator iter = keyword_list.find(identifier);
  token->reset(iter == keyword_list.end() ? Token::CreateId(location, identifier)
                                          : Token::Create(iter->second, location));
  if (token == nullptr) {
    return 0;
  }
  return advance;
}

// Parses a single token, and returns the advance delta of line offset.
size_t Lexer::ParseToken(size_t offset, unique_ptr<Token>* token) {
#define create_token(token_type, length) \
  do { \
    token->reset(Token::Create(token_type, Location(offset, offset + length))); \
    if (token == nullptr) { \
      return 0; \
    } \
    return length; \
  } while (false)

  // Handle single char token.
  const unordered_map<char, TokenType>::const_iterator iter = onechar_list.find(input_[offset]);
  if (iter != onechar_list.end()) {
    create_token(iter->second, 1);
  }

  // Handle multiple char token, natural number, identifier.
  if (offset + 1 < input_.length() && input_[offset] == '-' && input_[offset + 1] == '>') {
    create_token(TokenType::Arrow, 2);
  }

#undef create_token

  if (isdigit(input_[offset])) {
    return ParseNumber(offset, token);
  }

  if (isalpha(input_[offset])) {
    return ParseIdentifer(offset, token);
  }

  return 0;
}

/* static */
Lexer* Lexer::Create(const string& input) {
  unique_ptr<Lexer> lexer(new Lexer(input));
  size_t offset = 0;
  unique_ptr<Token> token;

  while (offset < lexer->input_.length()) {
    // Skip white spaces.
    if (is_whitespaces(lexer->input()[offset])) {
      ++offset;
      continue;
    }
    const int advance = lexer->ParseToken(offset, &token);
    if (advance == 0) {
      return nullptr;
    } else {
      lexer->tokens_.push_back(std::move(token));
      offset += advance;
    }
  }

  return lexer.release();
}
