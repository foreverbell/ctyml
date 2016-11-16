#include "parser.h"

#include <string>

#include "lexer.h"

using std::string;
using std::unique_ptr;
using std::vector;

using PatternPtr = unique_ptr<Pattern>;
using StmtPtr = unique_ptr<Stmt>;
using TermPtr = unique_ptr<Term>;
using TermTypePtr = unique_ptr<TermType>;

namespace {
namespace LL {

#define check_or_return(v) \
  do { \
    if (v == nullptr) { \
      return nullptr; \
    } \
  } while (false)

#define pop_or_return(token) \
  do { \
    if (!lexer->peak() || lexer->peak()->type() != token) return nullptr; \
    lexer->pop(); \
  } while (false)

#define pop_ucid_or_return(to) \
  do { \
    if (!lexer->peak() || lexer->peak()->type() != TokenType::UCaseId) return nullptr; \
  } while (false); \
  to = lexer->pop()->identifier()

#define pop_lcid_or_return(to) \
  do { \
    if (!lexer->peak() || lexer->peak()->type() != TokenType::LCaseId) return nullptr; \
  } while (false); \
  to = lexer->pop()->identifier()

#define pop_int_or_return(to) \
  do { \
    if (!lexer->peak() || lexer->peak()->type() != TokenType::Int) return nullptr; \
  } while (false); \
  to = lexer->pop()->number()

StmtPtr Statement(LexerIterator*);
TermPtr Term(LexerIterator*);
PatternPtr Pattern(LexerIterator*);
TermTypePtr Type(LexerIterator*);

StmtPtr Statement(LexerIterator* lexer) {
  const Token* token = lexer->peak();

  check_or_return(token);
  switch (token->type()) {
    case TokenType::TypeAlias: {
      lexer->pop();
      pop_ucid_or_return(const string& ucid);
      pop_or_return(TokenType::Eq);
      TermTypePtr type = Type(lexer);
      check_or_return(type);
      Location location(token, type.get());
      return StmtPtr(new BindTypeStmt(location, ucid, type.release()));
    }
    case TokenType::Let: {
      lexer->pop();
      PatternPtr pattern = Pattern(lexer);
      check_or_return(pattern);
      pop_or_return(TokenType::Eq);
      TermPtr term = Term(lexer);
      check_or_return(term);
      Location location(token, term.get());
      return StmtPtr(new BindTermStmt(location, pattern.release(), term.release()));
    }
    case TokenType::Letrec: {
      lexer->pop();
      // TODO(foreverbell): implement recursion.
      return nullptr;
    }
    // TODO(foreverbell): buggy, should try Term first.
    default: {
      TermPtr term = Term(lexer);
      check_or_return(term);
      Location location = term->location();
      return StmtPtr(new EvalStmt(location, term.release()));
    }
  }
  return nullptr;
}

TermPtr Term(LexerIterator* lexer) {
  const Token* token = lexer->peak();

  check_or_return(token);
  return nullptr;
}

PatternPtr Pattern(LexerIterator* lexer) {
  return nullptr;
}

TermTypePtr Type(LexerIterator* lexer) {
  return nullptr;
}

}  // namespace LL
}  // namespace

vector<StmtPtr> Parser::ParseAST() {
  vector<StmtPtr> stmts;

  return stmts;
}
