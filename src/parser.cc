#include "parser.h"

#include <string>

#include "context.h"
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

#define assign(v, expr) \
  do { \
    try { \
      v = expr; \
    } catch (ast_exception e) { \
      throw ast_exception(std::move(e), CFG); \
    } \
  } while (false)

#define assign_or_throw(v, expr) \
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

StmtPtr Statement(LexerIterator*, Context*);
PatternPtr Pattern(LexerIterator*, Context*);
TypedBindersPtr TypedBinders(LexerIterator*, Context*);
TermTypePtr Type(LexerIterator*, Context*);
TermPtr Term(LexerIterator*, Context*);

StmtPtr Statement(LexerIterator* lexer, Context* ctx) {
  const Token* token = lexer->peak();
  if (token == nullptr) return nullptr;

  switch (token->type()) {
    case TokenType::TypeAlias: {
      declare_cfg(R"(Statement = 'type' ucid '=' Type;)");
      TermTypePtr type;

      lexer->pop();
      pop_ucid_or_throw(const string& ucid);
      pop_or_throw(TokenType::Eq);
      assign_or_throw(type, Type(lexer, ctx));
      pop_or_throw(TokenType::Semi);
      return StmtPtr(new BindTypeStmt(Location(token->location(), lexer->last()->location()), ucid, type.release()));
    }
    case TokenType::Let: {
      declare_cfg(R"(Statement = 'let' Pattern '=' Term;)");
      PatternPtr pattern;
      TermPtr term;

      lexer->pop();
      assign_or_throw(pattern, Pattern(lexer, ctx));
      pop_or_throw(TokenType::Eq);
      assign_or_throw(term, Term(lexer, ctx));

      if (lexer->peak() == nullptr || lexer->peak()->type() == TokenType::Semi) {
        pop_or_throw(TokenType::Semi);
        return StmtPtr(new BindTermStmt(Location(token->location(), lexer->last()->location()),
                                        pattern.release(), term.release()));
      } else {
        declare_cfg(R"(Statement = Term;)");
        TermPtr stmt_term;
        assign_or_throw(stmt_term, [&]() -> TermPtr {
          declare_cfg(R"(Term = 'let' Pattern '=' Term 'in' Term)");
          TermPtr body;
          pop_or_throw(TokenType::In);
          assign_or_throw(body, Term(lexer, ctx));
          return TermPtr(new LetTerm(Location(token->location(), lexer->last()->location()),
                                     pattern.release(), term.release(), body.release()));
        }());
        pop_or_throw(TokenType::Semi);
        return StmtPtr(new EvalStmt(Location(token->location(), lexer->last()->location()), stmt_term.release())) ;
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

      assign_or_throw(term, Term(lexer, ctx));
      Location location = term->location();
      return StmtPtr(new EvalStmt(location, term.release()));
    }
  }
  return nullptr;
}

PatternPtr Pattern(LexerIterator* lexer, Context* ctx) {
  return nullptr;
}

TypedBindersPtr TypedBinder(LexerIterator* lexer, Context* ctx) {
  const Token* token = lexer->peak();
  if (token == nullptr) return nullptr;

  TermTypePtr type;
  TypedBindersPtr binder;

  if (token->type() == TokenType::LCaseId) {
    declare_cfg(R"(TypedBinder = lcid ':' Type)");

    pop_lcid_or_throw(const string& lcid);
    pop_or_throw(TokenType::Colon);
    assign_or_throw(type, Type(lexer, ctx));
    binder = TypedBindersPtr(new class TypedBinders(Location(token, type.get())));
    binder->add(lcid, type.release());
    ctx->AddName(lcid);
    return binder;
  } else if (token->type() == TokenType::UScore) {
    declare_cfg(R"(TypedBinder = '_' ':' Type)");

    pop_or_throw(TokenType::UScore);
    pop_or_throw(TokenType::Colon);
    assign_or_throw(type, Type(lexer, ctx));
    binder = TypedBindersPtr(new class TypedBinders(Location(token, type.get())));
    binder->add("_", type.release());
    ctx->AddName("_");
    return binder;
  }
  return nullptr;
}

TypedBindersPtr TypedBinders(LexerIterator* lexer, Context* ctx) {
  declare_cfg(R"(TypedBinders = TypedBinder | TypedBinder TypedBinders)");

  TypedBindersPtr binders = nullptr, cur;
  while (true) {
    assign(cur, TypedBinder(lexer, ctx));
    if (cur == nullptr) break;
    if (binders == nullptr) {
      binders = std::move(cur);
    } else {
      binders->relocate(Location(binders.get(), cur.get()));
      binders->merge(std::move(*cur.release()));
    }
  }
  return binders;
}

// Type = ArrowType
//
// ArrowType = AtomicType '->' ArrowType
//           | AtomicType
//
// AtomicType = '(' Type ')'
//            | 'Bool'
//            | 'Nat'
//            | 'List' '[' Type ']'
//            | 'Unit'
//            | '{' FieldTypes '}'
//            | ucid
//
// FieldTypes = FieldType
//            | FieldType ',' FieldTypes
//
// FieldType = lcid ':' Type

TermTypePtr FieldTypes(LexerIterator* lexer, Context* ctx) {
  auto FieldType = [](LexerIterator* lexer, Context* ctx) -> unique_ptr<RecordTermType> {
    declare_cfg(R"(FieldType = lcid ':' Type)");
    const Token* token = lexer->peak();
    if (token == nullptr || token->type() != TokenType::LCaseId) {
      return nullptr;
    }
    TermTypePtr type;
    unique_ptr<RecordTermType> field;

    pop_lcid_or_throw(const string& lcid);
    pop_or_throw(TokenType::Colon);
    assign_or_throw(type, Type(lexer, ctx));
    field = std::make_unique<RecordTermType>(Location(token, type.get()));
    field->add(lcid, type.release());
    return field;
  };

  declare_cfg(R"(FieldTypes = FieldType | FieldType ',' FieldTypes)");
  std::unique_ptr<RecordTermType> fields, cur;

  assign(fields, FieldType(lexer, ctx));
  if (fields == nullptr) return nullptr;
  while (lexer->peak() != nullptr && lexer->peak()->type() == TokenType::Comma) {
    pop_or_throw(TokenType::Comma);
    assign_or_throw(cur, FieldType(lexer, ctx));
    fields->relocate(Location(fields.get(), cur.get()));
    fields->merge(std::move(*cur.release()));
  }
  return TermTypePtr(fields.release());
}

TermTypePtr AtomicType(LexerIterator* lexer, Context* ctx) {
  const Token* token = lexer->peak();
  if (token == nullptr) return nullptr;

  switch (token->type()) {
    case TokenType::LParen: {
      declare_cfg(R"(AtomicType = '(' Type ')')");
      TermTypePtr type;

      pop_or_throw(TokenType::LParen);
      assign_or_throw(type, Type(lexer, ctx));
      pop_or_throw(TokenType::RParen);
      type->relocate(Location(token->location(), lexer->last()->location()));
      return type;
    }
    case TokenType::Bool: {
      declare_cfg(R"(AtomicType = 'Bool')");
      pop_or_throw(TokenType::Bool);
      return TermTypePtr(new BoolTermType(token->location()));
    }
    case TokenType::Nat: {
      declare_cfg(R"(AtomicType = 'Nat')");
      pop_or_throw(TokenType::Nat);
      return TermTypePtr(new NatTermType(token->location()));
    }
    case TokenType::Unit: {
      declare_cfg(R"(AtomicType = 'Unit')");
      pop_or_throw(TokenType::Unit);
      return TermTypePtr(new UnitTermType(token->location()));
    }
    case TokenType::List: {
      declare_cfg(R"(AtomicType ='List' '[' Type ']')");
      TermTypePtr type;

      pop_or_throw(TokenType::List);
      pop_or_throw(TokenType::LBracket);
      assign_or_throw(type, Type(lexer, ctx));
      pop_or_throw(TokenType::RBracket);
      return TermTypePtr(new ListTermType(Location(token->location(), lexer->last()->location()), type.release()));
    }
    case TokenType::LCurly: {
      declare_cfg(R"(AtomicType = {' FieldTypes '}')");
      TermTypePtr type;

      pop_or_throw(TokenType::LCurly);
      assign_or_throw(type, FieldTypes(lexer, ctx));
      pop_or_throw(TokenType::RCurly);
      type->relocate(Location(token->location(), lexer->last()->location()));
      return type;
    }
    case TokenType::UCaseId: {
      declare_cfg(R"(AtomicType = ucid)");
      pop_ucid_or_throw(const string& ucid);
      int index = ctx->ToIndex(ucid);
      if (index == -1) throw ast_exception(lexer->last()->location(), CFG, "type <" + ucid + "> is not found");
      return TermTypePtr(new UserDefinedType(lexer->last()->location(), index));
    }
    default: {
      return nullptr;
    }
  }
}

TermTypePtr ArrowType(LexerIterator* lexer, Context* ctx) {
  declare_cfg(R"(ArrowType = AtomicType)");

  TermTypePtr type1, type2;
  assign(type1, AtomicType(lexer, ctx));
  if (type1 == nullptr) return nullptr;

  const Token* token = lexer->peak();
  if (token == nullptr || token->type() != TokenType::Arrow) {
    return type1;
  } else {
    declare_cfg(R"(ArrowType = AtomicType '->' ArrowType)");
    pop_or_throw(TokenType::Arrow);
    assign_or_throw(type2, ArrowType(lexer, ctx));
    Location location(type1.get(), type2.get());
    return TermTypePtr(new ArrowTermType(location, type1.release(), type2.release()));
  }
}

TermTypePtr Type(LexerIterator* lexer, Context* ctx) {
  declare_cfg(R"(Type = ArrowType)");

  TermTypePtr type;
  assign(type, ArrowType(lexer, ctx));
  return type;
}

// Term = AppTerm
//      | 'lambda' TypedBinders '.' Term
//      | 'if' Term 'then' Term 'else' Term
//      | 'let' Pattern '=' Term 'in' Term
//      | 'letrec' TypedBinder '=' Term 'in' Term

TermPtr Term(LexerIterator* lexer, Context* ctx) {
  const Token* token = lexer->peak();
  if (token == nullptr) return nullptr;

  switch (token->type()) {
    case TokenType::Lambda: {
      declare_cfg(R"(Term = 'lambda' TypedBinders '.' Term)");
      TypedBindersPtr binders;
      TermPtr term;

      lexer->pop();
      assign_or_throw(binders, TypedBinders(lexer, ctx));
      pop_or_throw(TokenType::Dot);
      assign_or_throw(term, Term(lexer, ctx));
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
      assign_or_throw(term1, Term(lexer, ctx));
      pop_or_throw(TokenType::Then);
      assign_or_throw(term2, Term(lexer, ctx));
      pop_or_throw(TokenType::Else);
      assign_or_throw(term3, Term(lexer, ctx));
      return TermPtr(new TernaryTerm(Location(token->location(), lexer->last()->location()), TernaryTermToken::If,
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
