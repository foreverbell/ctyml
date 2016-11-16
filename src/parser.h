#pragma once

class Lexer;

class Parser {
 public:
  Parser(const Lexer* lexer) : lexer_(lexer) { }
  Parser(const Parser&) = delete;
  Parser& operator=(const Parser&) = delete;

  const Lexer* lexer() const { return lexer_; }

 private:
  const Lexer* const lexer_;  // not owned.
};
