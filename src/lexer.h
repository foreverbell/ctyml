#pragma once

#include <memory>
#include <string>
#include <vector>

#include "location.h"
#include "token.h"

class Lexer {
 public:
  Lexer() = delete;
  Lexer(const Lexer&) = delete;
  Lexer& operator=(const Lexer&) = delete;

  static Lexer* Create(const std::string& input);

  const std::string& input() const { return input_; }
  size_t size() const { return tokens_.size(); }
  const Token* get(int index) const { return tokens_.at(index).get(); }

 private:
  Lexer(const std::string& input) : input_(input), locator_(input) { }

  size_t ParseNumber(size_t offset, std::unique_ptr<Token>* token);
  size_t ParseIdentifer(size_t offset, std::unique_ptr<Token>* token);
  size_t ParseToken(size_t offset, std::unique_ptr<Token>* token);

  const std::string input_;
  Locator locator_;
  std::vector<std::unique_ptr<Token>> tokens_;
};
