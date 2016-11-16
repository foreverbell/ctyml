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
using TypedBindersPtr = unique_ptr<TypedBinders>;

namespace {
namespace LL {

#define declare_cfg(cfg) \
  static const string CFG = cfg

#define not_null_or_throw(v, expr) \
  do { \
    try { \
      v = expr; \
    } catch (ast_exception e) { \
      throw ast_exception(std::move(e), CFG); \
    } \
    if ((v) == nullptr) { \
      throw ast_exception(lexer->location(), CFG); \
    } \
  } while (false)

#define pop_or_throw(token) \
  do { \
    if (!lexer->peak() || lexer->peak()->type() != token) { \
      throw ast_exception(lexer->location(), CFG); \
    } \
    lexer->pop(); \
  } while (false)

#define pop_ucid_or_throw(to) \
  do { \
    if (!lexer->peak() || lexer->peak()->type() != TokenType::UCaseId) { \
      throw ast_exception(lexer->location(), CFG); \
    } \
  } while (false); \
  to = lexer->pop()->identifier()

#define pop_lcid_or_throw(to) \
  do { \
    if (!lexer->peak() || lexer->peak()->type() != TokenType::LCaseId) { \
      throw ast_exception(lexer->location(), CFG); \
    } \
  } while (false); \
  to = lexer->pop()->identifier()

#define pop_int_or_throw(to) \
  do { \
    if (!lexer->peak() || lexer->peak()->type() != TokenType::Int) { \
      throw ast_exception(lexer->location(), CFG); \
    } \
  } while (false); \
  to = lexer->pop()->number()

// Only throws exception if commits to this branch, i.e. some tokens are consumed.

StmtPtr Statement(LexerIterator*);
PatternPtr Pattern(LexerIterator*);
TypedBindersPtr TypedBinders(LexerIterator*);
TermTypePtr Type(LexerIterator*);
TermPtr Term(LexerIterator*);

StmtPtr Statement(LexerIterator* lexer) {
  const Token* token = lexer->peak();
  if (token == nullptr) {
    return nullptr;
  }

  switch (token->type()) {
    case TokenType::TypeAlias: {
      declare_cfg(R"(Statement = 'type' ucid '=' Type;)");
      TermTypePtr type;

      lexer->pop();
      pop_ucid_or_throw(const string& ucid);
      pop_or_throw(TokenType::Eq);
      not_null_or_throw(type, Type(lexer));
      pop_or_throw(TokenType::Semi);
      Location location(token->location(), lexer->last()->location());
      return StmtPtr(new BindTypeStmt(location, ucid, type.release()));
    }
    case TokenType::Let: {
      declare_cfg(R"(Statement = 'let' Pattern '=' Term;)");
      PatternPtr pattern;
      TermPtr term;

      lexer->pop();
      not_null_or_throw(pattern, Pattern(lexer));
      pop_or_throw(TokenType::Eq);
      not_null_or_throw(term, Term(lexer));

      if (lexer->peak() == nullptr || lexer->peak()->type() == TokenType::Semi) {
        pop_or_throw(TokenType::Semi);
        Location location(token->location(), lexer->last()->location());
        return StmtPtr(new BindTermStmt(location, pattern.release(), term.release()));
      } else {
        declare_cfg(R"(Statement = Term;)");
        TermPtr stmt_term;
        not_null_or_throw(stmt_term, [&]() -> TermPtr {
          declare_cfg(R"(Term = 'let' Pattern '=' Term 'in' Term)");
          TermPtr body;
          pop_or_throw(TokenType::In);
          not_null_or_throw(body, Term(lexer));
          Location location(token, body.get());
          return TermPtr(new LetTerm(location, pattern.release(), term.release(), body.release()));
        }());
        pop_or_throw(TokenType::Semi);
        Location location(token->location(), lexer->last()->location());
        return StmtPtr(new EvalStmt(location, stmt_term.release())) ;
      }
    }
    case TokenType::Letrec: {
      lexer->pop();
      // TODO(foreverbell): implement recursion.
      return nullptr;
    }
    default: {
      declare_cfg(R"(Statement = Term)");
      TermPtr term;

      not_null_or_throw(term, Term(lexer));
      Location location = term->location();
      return StmtPtr(new EvalStmt(location, term.release()));
    }
  }
  return nullptr;
}

PatternPtr Pattern(LexerIterator* lexer) {
  return nullptr;
}

TypedBindersPtr TypedBinders(LexerIterator* lexer) {
  return nullptr;
}

TermTypePtr Type(LexerIterator* lexer) {
  return nullptr;
}

// Term = AppTerm
//      | 'lambda' TypedBinders '.' Term
//      | 'if' Term 'then' Term 'else' Term
//      | 'let' Pattern '=' Term 'in' Term
//      | 'letrec' TypedBinder '=' Term 'in' Term

TermPtr Term(LexerIterator* lexer) {
  const Token* token = lexer->peak();
  if (token == nullptr) {
    return nullptr;
  }

  switch (token->type()) {
    case TokenType::Lambda: {
      declare_cfg(R"(Term = 'lambda' TypedBinders '.' Term)");
      TypedBindersPtr binders;
      TermPtr term;

      lexer->pop();
      not_null_or_throw(binders, TypedBinders(lexer));
      pop_or_throw(TokenType::Dot);
      not_null_or_throw(term, Term(lexer));
      Location location(token, term.get());
      for (int i = int(binders->size()) - 1; i >= 0; --i) {
        term = TermPtr(new AbsTerm(location, binders->get(i).first, binders->get(i).second.release(), term.release()));
      }
      return term;
    }
    case TokenType::If: {
      declare_cfg(R"(Term = 'if' Term 'then' Term 'else' Term)");
      TermPtr term1, term2, term3;

      lexer->pop();
      not_null_or_throw(term1, Term(lexer));
      pop_or_throw(TokenType::Then);
      not_null_or_throw(term2, Term(lexer));
      pop_or_throw(TokenType::Else);
      not_null_or_throw(term3, Term(lexer));
      Location location(token, term3.get());
      return TermPtr(new TernaryTerm(location, TernaryTermToken::If,
                                     term1.release(), term2.release(), term3.release()));
    }

  }
  return nullptr;
}

}  // namespace LL
}  // namespace

vector<StmtPtr> Parser::ParseAST() {
  vector<StmtPtr> stmts;

  return stmts;
}
