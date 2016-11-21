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

#define cfg_scope(cfg) \
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
      string err(#expr); \
      err = err.substr(0, err.find("(")); \
      throw ast_exception(lexer->location(), CFG, "expect a " + err); \
    } \
  } while (false)

#define pop_or_throw(token) \
  do { \
    if (!lexer->peak() || lexer->peak()->type() != token) { \
      string err(#token); \
      err = err.substr(err.find_last_of("::") + 1); \
      throw ast_exception(lexer->location(), CFG, "expect a " + err); \
    } \
    lexer->pop(); \
  } while (false)

#define pop_ucid_or_throw(to) \
  do { \
    if (!lexer->peak() || lexer->peak()->type() != TokenType::UCaseId) { \
      throw ast_exception(lexer->location(), CFG, "expect an ucid"); \
    } \
  } while (false); \
  to = lexer->pop()->identifier()

#define pop_lcid_or_throw(to) \
  do { \
    if (!lexer->peak() || lexer->peak()->type() != TokenType::LCaseId) { \
      throw ast_exception(lexer->location(), CFG, "expect a lcid"); \
    } \
  } while (false); \
  to = lexer->pop()->identifier()

#define pop_int_or_throw(to) \
  do { \
    if (!lexer->peak() || lexer->peak()->type() != TokenType::Int) { \
      throw ast_exception(lexer->location(), CFG, "expect a natural number"); \
    } \
  } while (false); \
  to = lexer->pop()->number()

namespace {
namespace LL {

// Dear bison & flex, you were right, salvation lays within.
//
// The grammar of our tiny PL is able to be parsed with an LL(1) parser.
// For failure of each function, only throws exception if parser commits to this branch,
// i.e. some tokens are consumed, otherwise just returns a null pointer.

StmtPtr Statement(LexerIterator*, Context*);
PatternPtr Pattern(LexerIterator*, Context*);
TypedBindersPtr TypedBinder(LexerIterator*, Context*);
TypedBindersPtr TypedBinders(LexerIterator*, Context*);
TermTypePtr Type(LexerIterator*, Context*);
TermPtr Term(LexerIterator*, Context*);

// Statement parsers.
//
// Statement = Term ';'
//           | 'type' ucid '=' Type ';'
//           | 'let' Pattern '=' Term ';'
//           | 'letrec' TypedBinder '=' Term ';'

StmtPtr Statement(LexerIterator* lexer, Context* ctx) {
  const Token* token = lexer->peak();
  if (token == nullptr) return nullptr;

  switch (token->type()) {
    case TokenType::TypeAlias: {
      cfg_scope(R"(Statement = 'type' ucid '=' Type ';')");
      TermTypePtr type;

      pop_or_throw(TokenType::TypeAlias);
      pop_ucid_or_throw(const string& ucid);
      pop_or_throw(TokenType::Eq);
      assign_or_throw(type, Type(lexer, ctx));
      pop_or_throw(TokenType::Semi);
      ctx->AddName(ucid);
      return StmtPtr(new BindTypeStmt(Location(token->location(), lexer->last_loc()), ucid, type.release()));
    }
    case TokenType::Let: {
      cfg_scope(R"(Statement = 'let' Pattern '=' Term ';')");
      PatternPtr pattern;
      TermPtr term;

      pop_or_throw(TokenType::Let);
      assign_or_throw(pattern, Pattern(lexer, ctx));
      pop_or_throw(TokenType::Eq);
      assign_or_throw(term, Term(lexer, ctx));

      if (lexer->peak() == nullptr || lexer->peak()->type() == TokenType::Semi) {
        pop_or_throw(TokenType::Semi);  // expect to fail if lexer->peak() == nullptr.
        return StmtPtr(new BindTermStmt(Location(token->location(), lexer->last_loc()),
                                        pattern.release(), term.release()));
      } else {
        cfg_scope(R"(Statement = Term ';')");
        TermPtr stmt_term;

        assign_or_throw(stmt_term, [&]() -> TermPtr {
          cfg_scope(R"(Term = 'let' Pattern '=' Term 'in' Term)");
          TermPtr body;

          pop_or_throw(TokenType::In);
          assign_or_throw(body, Term(lexer, ctx));
          ctx->DropBindings(1);  // throws away the binding introduced in Pattern.
          return TermPtr(new LetTerm(Location(token->location(), lexer->last_loc()),
                                     pattern.release(), term.release(), body.release()));
        }());
        pop_or_throw(TokenType::Semi);
        return StmtPtr(new EvalStmt(Location(token->location(), lexer->last_loc()), stmt_term.release()));
      }
    }
    case TokenType::LetRec: {
      cfg_scope(R"(Statement = 'letrec' TypedBinder '=' Term ';')");
      TypedBindersPtr binders;
      TermPtr term;

      pop_or_throw(TokenType::LetRec);
      assign_or_throw(binders, TypedBinder(lexer, ctx));
      assert(binders->size() == 1);
      pop_or_throw(TokenType::Eq);
      assign_or_throw(term, Term(lexer, ctx));

      if (lexer->peak() == nullptr || lexer->peak()->type() == TokenType::Semi) {
        pop_or_throw(TokenType::Semi);  // expect to fail if lexer->peak() == nullptr.

        const std::string& variable = binders->get(0).first;
        PatternPtr pattern(new class Pattern(binders->location(), variable));
        Location location = Location(binders.get(), term.get());
        TermPtr fix_term(
            new UnaryTerm(location, UnaryTermToken::Fix,
              new AbsTerm(location, variable, binders->get(0).second.release(), term.release())));
        return StmtPtr(new BindTermStmt(Location(token->location(), lexer->last_loc()),
                                        pattern.release(), fix_term.release()));
      } else {
        cfg_scope(R"(Statement = Term ';')");
        TermPtr stmt_term;

        assign_or_throw(stmt_term, [&]() -> TermPtr {
          cfg_scope(R"(Term = 'letrec' TypedBinder '=' Term 'in' Term)");
          TermPtr body;

          pop_or_throw(TokenType::In);
          assign_or_throw(body, Term(lexer, ctx));
          ctx->DropBindings(1);  // throws away the binding introduced in TypedBinder.

          const std::string& variable = binders->get(0).first;
          PatternPtr pattern(new class Pattern(binders->location(), variable));
          Location location = Location(binders.get(), term.get());
          TermPtr fix_term(
              new UnaryTerm(location, UnaryTermToken::Fix,
                new AbsTerm(location, variable, binders->get(0).second.release(), term.release())));
          return TermPtr(new LetTerm(Location(token->location(), lexer->last_loc()),
                                     pattern.release(), fix_term.release(), body.release()));
        }());
        pop_or_throw(TokenType::Semi);
        return StmtPtr(new EvalStmt(Location(token->location(), lexer->last_loc()), stmt_term.release()));
      }
      return nullptr;
    }
    default: {
      cfg_scope(R"(Statement = Term ';')");
      TermPtr term;

      assign(term, Term(lexer, ctx));
      if (term == nullptr) return nullptr;
      pop_or_throw(TokenType::Semi);
      Location location(term->location(), lexer->last_loc());
      return StmtPtr(new EvalStmt(location, term.release()));
    }
  }
}

// Pattern parsers.
//
// Pattern = lcid
//         | '_'

PatternPtr Pattern(LexerIterator* lexer, Context* ctx) {
  const Token* token = lexer->peak();
  if (token == nullptr) return nullptr;

  if (token->type() == TokenType::LCaseId) {
    cfg_scope(R"(Pattern = lcid)");

    pop_lcid_or_throw(const string& lcid);
    ctx->AddName(lcid);
    return PatternPtr(new class Pattern(token->location(), lcid));
  } else if (token->type() == TokenType::UScore) {
    cfg_scope(R"(Pattern = '_')");

    pop_or_throw(TokenType::UScore);
    ctx->AddName("_");
    return PatternPtr(new class Pattern(token->location(), "_"));
  }
  return nullptr;
}

// TypedBinders parsers.
//
// TypedBinders = TypedBinder
//              | TypedBinder TypedBinders
//
// TypedBinder = lcid ':' Type
// 	       | '_' ':' Type

TypedBindersPtr TypedBinder(LexerIterator* lexer, Context* ctx) {
  const Token* token = lexer->peak();
  if (token == nullptr) return nullptr;

  TermTypePtr type;
  TypedBindersPtr binder;

  if (token->type() == TokenType::LCaseId) {
    cfg_scope(R"(TypedBinder = lcid ':' Type)");

    pop_lcid_or_throw(const string& lcid);
    pop_or_throw(TokenType::Colon);
    assign_or_throw(type, Type(lexer, ctx));
    binder = TypedBindersPtr(new class TypedBinders(Location(token, type.get())));
    binder->add(lcid, type.release());
    ctx->AddName(lcid);
    return binder;
  } else if (token->type() == TokenType::UScore) {
    cfg_scope(R"(TypedBinder = '_' ':' Type)");

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
  cfg_scope(R"(TypedBinders = TypedBinder | TypedBinder TypedBinders)");

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

// Type parsers.
//
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
    cfg_scope(R"(FieldType = lcid ':' Type)");
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

  cfg_scope(R"(FieldTypes = FieldType | FieldType ',' FieldTypes)");
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
      cfg_scope(R"(AtomicType = '(' Type ')')");
      TermTypePtr type;

      pop_or_throw(TokenType::LParen);
      assign_or_throw(type, Type(lexer, ctx));
      pop_or_throw(TokenType::RParen);
      type->relocate(Location(token->location(), lexer->last_loc()));
      return type;
    }
    case TokenType::Bool: {
      cfg_scope(R"(AtomicType = 'Bool')");

      pop_or_throw(TokenType::Bool);
      return TermTypePtr(new BoolTermType(token->location()));
    }
    case TokenType::Nat: {
      cfg_scope(R"(AtomicType = 'Nat')");

      pop_or_throw(TokenType::Nat);
      return TermTypePtr(new NatTermType(token->location()));
    }
    case TokenType::UUnit: {
      cfg_scope(R"(AtomicType = 'Unit')");

      pop_or_throw(TokenType::UUnit);
      return TermTypePtr(new UnitTermType(token->location()));
    }
    case TokenType::List: {
      cfg_scope(R"(AtomicType ='List' '[' Type ']')");
      TermTypePtr type;

      pop_or_throw(TokenType::List);
      pop_or_throw(TokenType::LBracket);
      assign_or_throw(type, Type(lexer, ctx));
      pop_or_throw(TokenType::RBracket);
      return TermTypePtr(new ListTermType(Location(token->location(), lexer->last_loc()), type.release()));
    }
    case TokenType::LCurly: {
      cfg_scope(R"(AtomicType = {' FieldTypes '}')");
      TermTypePtr type;

      pop_or_throw(TokenType::LCurly);
      assign_or_throw(type, FieldTypes(lexer, ctx));
      pop_or_throw(TokenType::RCurly);
      type->relocate(Location(token->location(), lexer->last_loc()));
      return type;
    }
    case TokenType::UCaseId: {
      cfg_scope(R"(AtomicType = ucid)");

      pop_ucid_or_throw(const string& ucid);
      int index = ctx->ToIndex(ucid);
      if (index == -1) throw ast_exception(lexer->last_loc(), CFG, "type <" + ucid + "> is not in scope");
      return TermTypePtr(new UserDefinedType(lexer->last_loc(), index));
    }
    default: {
      return nullptr;
    }
  }
}

TermTypePtr ArrowType(LexerIterator* lexer, Context* ctx) {
  cfg_scope(R"(ArrowType = AtomicType)");

  TermTypePtr type1, type2;
  assign(type1, AtomicType(lexer, ctx));
  if (type1 == nullptr) return nullptr;

  const Token* token = lexer->peak();
  if (token == nullptr || token->type() != TokenType::Arrow) {
    return type1;
  } else {
    cfg_scope(R"(ArrowType = AtomicType '->' ArrowType)");

    pop_or_throw(TokenType::Arrow);
    assign_or_throw(type2, ArrowType(lexer, ctx));
    Location location(type1.get(), type2.get());
    return TermTypePtr(new ArrowTermType(location, type1.release(), type2.release()));
  }
}

TermTypePtr Type(LexerIterator* lexer, Context* ctx) {
  cfg_scope(R"(Type = ArrowType)");

  TermTypePtr type;
  assign(type, ArrowType(lexer, ctx));
  return type;
}

// Term parsers.
//
// Term = AppTerm
//      | 'lambda' TypedBinders '.' Term
//      | 'if' Term 'then' Term 'else' Term
//      | 'let' Pattern '=' Term 'in' Term
//      | 'letrec' TypedBinder '=' Term 'in' Term
//
// AppTerm = PathTerm
//         | 'succ' PathTerm
//         | 'pred' PathTerm
//         | 'iszero' PathTerm
//         | 'cons' PathTerm PathTerm
//         | 'isnil' PathTerm
//         | 'head' PathTerm
//         | 'tail' PathTerm
//         | AppTerm PathTerm
//
// PathTerm = PathTerm '.' lcid
//          | AscribeTerm
//
// AscribeTerm = AtomicTerm
//             | AtomicTerm 'as' Type
//
// AtomicTerm = '(' Term ')'
//            | 'true'
//            | 'false'
//            | int
//            | 'nil' '[' Type ']'
//            | 'unit'
//            | '{' Fields '}'
//            | lcid
//
// Fields = Field
//        | Field ',' Fields
//
// Field = lcid '=' Term

TermPtr Fields(LexerIterator* lexer, Context* ctx) {
  auto Field = [](LexerIterator* lexer, Context* ctx) -> unique_ptr<RecordTerm> {
    cfg_scope(R"(Field = lcid '=' Term)");
    const Token* token = lexer->peak();
    if (token == nullptr || token->type() != TokenType::LCaseId) {
      return nullptr;
    }
    TermPtr term;
    unique_ptr<RecordTerm> field;

    pop_lcid_or_throw(const string& lcid);
    pop_or_throw(TokenType::Colon);
    assign_or_throw(term, Term(lexer, ctx));
    field = std::make_unique<RecordTerm>(Location(token, term.get()));
    field->add(lcid, term.release());
    return field;
  };

  cfg_scope(R"(Fields = Field | Field ',' Fields)");
  std::unique_ptr<RecordTerm> fields, cur;

  assign(fields, Field(lexer, ctx));
  if (fields == nullptr) return nullptr;
  while (lexer->peak() != nullptr && lexer->peak()->type() == TokenType::Comma) {
    pop_or_throw(TokenType::Comma);
    assign_or_throw(cur, Field(lexer, ctx));
    fields->relocate(Location(fields.get(), cur.get()));
    fields->merge(std::move(*cur.release()));
  }
  return TermPtr(fields.release());
}

TermPtr AtomicTerm(LexerIterator* lexer, Context* ctx) {
  const Token* token = lexer->peak();
  if (token == nullptr) return nullptr;

  switch (token->type()) {
    case TokenType::LParen: {
      cfg_scope(R"(AtomicTerm = '(' Term ')')");
      TermPtr term;

      pop_or_throw(TokenType::LParen);
      assign_or_throw(term, Term(lexer, ctx));
      pop_or_throw(TokenType::RParen);
      term->relocate(Location(token->location(), lexer->last_loc()));
      return term;
    }
    case TokenType::True: {
      cfg_scope(R"(AtomicTerm = 'true')");

      pop_or_throw(TokenType::True);
      return TermPtr(new NullaryTerm(token->location(), NullaryTermToken::True));
    }
    case TokenType::False: {
      cfg_scope(R"(AtomicTerm = 'false')");

      pop_or_throw(TokenType::False);
      return TermPtr(new NullaryTerm(token->location(), NullaryTermToken::False));
    }
    case TokenType::Int: {
      cfg_scope(R"(AtomicTerm = int)");
      TermPtr term;

      pop_int_or_throw(int number);
      assign_or_throw(term, NullaryTerm::CreateInt(token->location(), number));
      return term;
    }
    case TokenType::Nil: {
      cfg_scope(R"(AtomicTerm = 'nil' '[' Type ']')");
      TermTypePtr type;

      pop_or_throw(TokenType::Nil);
      pop_or_throw(TokenType::LBracket);
      assign_or_throw(type, Type(lexer, ctx));
      pop_or_throw(TokenType::RBracket);
      return TermPtr(new NilTerm(Location(token->location(), lexer->last_loc()), type.release()));
    }
    case TokenType::Unit: {
      cfg_scope(R"(AtomicTerm = 'unit')");

      pop_or_throw(TokenType::Unit);
      return TermPtr(new NullaryTerm(token->location(), NullaryTermToken::Unit));
    }
    case TokenType::LCurly: {
      cfg_scope(R"(AtomicTerm = '{' Fields '}')");
      TermPtr term;

      pop_or_throw(TokenType::LCurly);
      assign_or_throw(term, Fields(lexer, ctx));
      pop_or_throw(TokenType::RCurly);
      term->relocate(Location(token->location(), lexer->last_loc()));
      return term;
    }
    case TokenType::LCaseId: {
      cfg_scope(R"(AtomicTerm = lcid)");

      pop_lcid_or_throw(const string& lcid);
      int index = ctx->ToIndex(lcid);
      if (index == -1) throw ast_exception(lexer->last_loc(), CFG, "variable <" + lcid + "> is not in scope");
      return TermPtr(new VariableTerm(lexer->last_loc(), index));
    }
    default: {
      return nullptr;
    }
  }
}

TermPtr AscribeTerm(LexerIterator* lexer, Context* ctx) {
  cfg_scope(R"(AscribeTerm = AtomicTerm)");
  TermPtr term;
  TermTypePtr type;

  assign(term, AtomicTerm(lexer, ctx));
  if (term == nullptr) return nullptr;
  if (lexer->peak() != nullptr && lexer->peak()->type() == TokenType::As) {
    cfg_scope(R"(AscribeTerm = AtomicTerm 'as' Type)");

    pop_or_throw(TokenType::As);
    assign_or_throw(type, Type(lexer, ctx));
    Location location(term.get(), type.get());
    return TermPtr(new class AscribeTerm(location, term.release(), type.release()));
  }
  return term;
}

// PathTerm introduces left recursion. The expansion of AppTerm can be viewed as
//   AscribeTerm [. lcid]*
TermPtr PathTerm(LexerIterator* lexer, Context* ctx) {
  cfg_scope(R"(PathTerm = AscribeTerm)");
  TermPtr term;

  assign(term, AscribeTerm(lexer, ctx));
  if (term == nullptr) return nullptr;
  while (lexer->peak() != nullptr && lexer->peak()->type() == TokenType::Dot) {
    cfg_scope(R"(PathTerm = PathTerm '.' lcid)");

    pop_or_throw(TokenType::Dot);
    pop_lcid_or_throw(const string& lcid);
    Location location(term->location(), lexer->last_loc());
    term.reset(new ProjectTerm(location, term.release(), lcid));
  }
  return term;
}

// AppTerm introduces left recursion. The expansion of AppTerm can be viewed as
//   {PathTerm, 'succ' PathTerm ...} [PathTerm]*
TermPtr AppTerm(LexerIterator* lexer, Context* ctx) {
  const Token* token = lexer->peak();
  if (token == nullptr) return nullptr;
  TermPtr term;

  switch (token->type()) {
#define unary_term(token_type, token_id) \
    case (TokenType::token_type): { \
      cfg_scope(R"(AppTerm = ')" #token_id R"(' PathTerm)"); \
      TermPtr path_term; \
       \
      pop_or_throw(TokenType::token_type); \
      assign_or_throw(path_term, PathTerm(lexer, ctx)); \
      Location location(token, path_term.get()); \
      term = TermPtr(new UnaryTerm(location, UnaryTermToken::token_type, path_term.release())); \
    } break

    unary_term(Succ, succ);
    unary_term(Pred, pred);
    unary_term(IsZero, iszero);
    unary_term(IsNil, isnil);
    unary_term(Head, head);
    unary_term(Tail, tail);

#undef unary_term

    case (TokenType::Cons): {
      cfg_scope(R"(AppTerm = 'cons' PathTerm PathTerm)");
      TermPtr term1, term2;

      pop_or_throw(TokenType::Cons);
      assign_or_throw(term1, PathTerm(lexer, ctx));
      assign_or_throw(term2, PathTerm(lexer, ctx));
      Location location(token, term2.get());
      term = TermPtr(new BinaryTerm(location, BinaryTermToken::Cons, term1.release(), term2.release()));
    } break;
    default: {
      cfg_scope(R"(AppTerm = PathTerm)");

      assign(term, PathTerm(lexer, ctx));
      if (term == nullptr) return nullptr;
    }
  }

  while (lexer->peak() != nullptr) {
    cfg_scope(R"(AppTerm = AppTerm PathTerm)");
    TermPtr path_term;

    assign(path_term, PathTerm(lexer, ctx));
    if (path_term == nullptr) break;
    Location location(term.get(), path_term.get());
    term = TermPtr(new BinaryTerm(location, BinaryTermToken::App, term.release(), path_term.release()));
  }
  return term;
}

TermPtr Term(LexerIterator* lexer, Context* ctx) {
  const Token* token = lexer->peak();
  if (token == nullptr) return nullptr;

  switch (token->type()) {
    case TokenType::Lambda: {
      cfg_scope(R"(Term = 'lambda' TypedBinders '.' Term)");
      TypedBindersPtr binders;
      TermPtr term;

      pop_or_throw(TokenType::Lambda);
      assign_or_throw(binders, TypedBinders(lexer, ctx));
      pop_or_throw(TokenType::Dot);
      assign_or_throw(term, Term(lexer, ctx));
      ctx->DropBindings(binders->size());  // throws away the binding introduced in TypedBinders.
      Location location(token, term.get());
      for (int i = int(binders->size()) - 1; i >= 0; --i) {
        term = TermPtr(new AbsTerm(location, binders->get(i).first, binders->get(i).second.release(), term.release()));
      }
      return term;
    }
    case TokenType::If: {
      cfg_scope(R"(Term = 'if' Term 'then' Term 'else' Term)");
      TermPtr term1, term2, term3;

      pop_or_throw(TokenType::If);
      assign_or_throw(term1, Term(lexer, ctx));
      pop_or_throw(TokenType::Then);
      assign_or_throw(term2, Term(lexer, ctx));
      pop_or_throw(TokenType::Else);
      assign_or_throw(term3, Term(lexer, ctx));
      return TermPtr(new TernaryTerm(Location(token->location(), lexer->last_loc()), TernaryTermToken::If,
                                     term1.release(), term2.release(), term3.release()));
    }
    case TokenType::Let: {
      cfg_scope(R"(Term = 'let' Pattern '=' Term 'in' Term)");
      PatternPtr pattern;
      TermPtr term, body;

      pop_or_throw(TokenType::Let);
      assign_or_throw(pattern, Pattern(lexer, ctx));
      pop_or_throw(TokenType::Eq);
      assign_or_throw(term, Term(lexer, ctx));
      pop_or_throw(TokenType::In);
      assign_or_throw(body, Term(lexer, ctx));
      ctx->DropBindings(1);  // throws away the binding introduced in Pattern.
      return TermPtr(new LetTerm(Location(token->location(), lexer->last_loc()),
                                 pattern.release(), term.release(), body.release()));
    }
    case TokenType::LetRec: {
      cfg_scope(R"(Term = 'letrec' TypedBinder '=' Term 'in' Term)");
      TypedBindersPtr binders;
      TermPtr term, body;

      pop_or_throw(TokenType::LetRec);
      assign_or_throw(binders, TypedBinder(lexer, ctx));
      assert(binders->size() == 1);
      pop_or_throw(TokenType::Eq);
      assign_or_throw(term, Term(lexer, ctx));
      pop_or_throw(TokenType::In);
      assign_or_throw(body, Term(lexer, ctx));
      ctx->DropBindings(1);  // throws away the binding introduced in TypedBinder.

      const std::string& variable = binders->get(0).first;
      PatternPtr pattern(new class Pattern(binders->location(), variable));
      Location location = Location(binders.get(), term.get());
      TermPtr fix_term(
          new UnaryTerm(location, UnaryTermToken::Fix,
            new AbsTerm(location, variable, binders->get(0).second.release(), term.release())));
      return TermPtr(new LetTerm(Location(token->location(), lexer->last_loc()),
                                 pattern.release(), fix_term.release(), body.release()));
    }
    default: {
      cfg_scope(R"(Term = AppTerm)");
      TermPtr term;

      assign(term, AppTerm(lexer, ctx));
      return term;
    }
  }
}

}  // namespace LL
}  // namespace

// Topmost = Statement
//         | Statement Topmost

vector<StmtPtr> Parser::ParseAST() {
  cfg_scope(R"(Topmost = Statement | Statement Topmost)");

  vector<StmtPtr> stmts;
  LexerIterator lexer_iter(lexer_);
  Context ctx;

  do {
    StmtPtr stmt;
    try {
      stmt = LL::Statement(&lexer_iter, &ctx);
    } catch (ast_exception e) {
      throw ast_exception(std::move(e), CFG);
    }
    if (stmt == nullptr) {
      throw ast_exception(lexer_iter.location(), CFG);
    }
    stmts.push_back(std::move(stmt));
  } while (!lexer_iter.eof());
  return stmts;
}
