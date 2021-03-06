#pragma once

#include <memory>
#include <vector>

#include "ast.h"

class Context;
class Lexer;

class Parser {
 public:
  Parser(const Lexer* lexer) : lexer_(lexer) { }
  Parser(const Parser&) = delete;
  Parser& operator=(const Parser&) = delete;

  const Lexer* lexer() const { return lexer_; }

  std::vector<std::unique_ptr<Stmt>> ParseAST(Context* ctx);

 private:
  const Lexer* const lexer_;  // not owned.
};
