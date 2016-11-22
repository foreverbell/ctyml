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

  // TODO(foreverbell): After lexcial_exception is implemented, move out creation logic from factory method.
  static Lexer* Create(const std::string& input);

  const std::string& input() const { return input_; }
  const Locator* locator() const { return &locator_; }

  size_t size() const { return tokens_.size(); }
  const Token* get(int index) const { return tokens_.at(index).get(); }

 private:
  Lexer(const std::string& input) : input_(input), locator_("(file)", input) { }

  size_t ParseNumber(size_t offset, std::unique_ptr<Token>* token);
  size_t ParseIdentifer(size_t offset, std::unique_ptr<Token>* token);
  size_t ParseToken(size_t offset, std::unique_ptr<Token>* token);

  const std::string input_;
  Locator locator_;
  std::vector<std::unique_ptr<Token>> tokens_;
};

class LexerIterator {
 public:
  LexerIterator(const Lexer* lexer) : lexer_(lexer), offset_(0) { }

  void reset() { offset_ = 0; }
  Location last_loc() const { assert(offset_ > 0); return lexer_->get(offset_ - 1)->location(); }
  const Token* peak() const { return eof() ? nullptr : lexer_->get(offset_); }
  const Token* pop() {
    const Token* cur = peak();
    if (cur == nullptr) return cur;
    ++offset_; return cur;
  }
  size_t size() const { return lexer_->size(); }
  bool eof() const { return offset_ >= size(); }

  Location location() const { return eof() ? Location(-1, -1) : peak()->location(); }

 private:
  const Lexer* const lexer_;
  int offset_;
};
