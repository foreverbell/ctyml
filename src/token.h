#pragma once

#include <cassert>
#include <memory>
#include <string>

#include "syntax.h"

class Token final {
 public:
  Token() = delete;
  Token(const Token&) = delete;

  static bool Create(TokenType type, Location location, std::unique_ptr<Token>* token);
  static bool CreateInt(Location location, int number, std::unique_ptr<Token>* token);
  static bool CreateId(Location location, const std::string& id, std::unique_ptr<Token>* token);

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

  TokenType type_ = TokenType::Undefined;
  Location location_;

  int number_ = -1;
  std::string identifier_;
};
