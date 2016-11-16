#pragma once

#include <cassert>
#include <string>

#include "location.h"

enum class TokenType {
  Int,               // With a natural integer number.
  LCaseId, UCaseId,  // With an alphabet-leading identifier.
  If, Then, Else,
  Succ, Pred, IsZero,
  True, False, Zero,
  Nil, Cons,
  IsNil, Head, Tail,
  Unit,
  Bool, Nat, List, UUnit,
  Lambda, Let, In, Letrec, TypeAlias, As,
  LParen, RParen,
  LCurly, RCurly,
  LBracket, RBracket,
  Arrow, Dot, Comma, Colon, Semi, Eq, UScore,
};

class Token final {
 public:
  Token() = delete;
  Token(const Token&) = delete;
  Token& operator=(const Token&) = delete;

  static Token* Create(TokenType type, Location location) {
    if (type == TokenType::Int || type == TokenType::LCaseId || type == TokenType::UCaseId) {
      return nullptr;
    }
    return new Token(type, location, -1, "");
  }

  static Token* CreateInt(Location location, int number) {
    if (number < 0) {
      return nullptr;
    }
    return new Token(TokenType::Int, location, number, "");
  }

  static Token* CreateId(Location location, const std::string& id) {
    if (id.empty() || !isalpha(id[0])) {
      return nullptr;
    }
    // Valid identifier should be led by an alphabet, followed by alphabets, digits, single-quotes or underscores.
    for (size_t i = 1; i < id.size(); ++i) {
      if (!isalpha(id[i]) && !isdigit(id[i]) && id[i] != '\'' && id[i] != '_') {
        return nullptr;
      }
    }
    return new Token(id[0] >= 'A' && id[0] <= 'Z' ? TokenType::UCaseId : TokenType::LCaseId, location, -1, id);
  }

  TokenType type() const { return type_; }
  Location location() const { return location_; }

  bool is_int() const { return type() == TokenType::Int; }
  bool is_id() const { return type() == TokenType::LCaseId || type() == TokenType::UCaseId; }

  int number() const {
    assert(is_int());
    return number_;
  }

  const std::string& identifier() const {
    assert(is_id());
    return identifier_;
  }

 private:
  Token(TokenType type, Location location, int number, const std::string& identifier)
    : type_(type), location_(location), number_(number), identifier_(identifier) { }

  const TokenType type_;
  const Location location_;

  const int number_;
  const std::string identifier_;
};
