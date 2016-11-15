#pragma once

// Context free grammar for our toy PL.
//
// Topmost = Statement ';'
//         | Statement ';' Topmost
//
// Statement = Term
//           | 'type' ucid '=' Type
//           | 'let' LetBinder
//           | 'letrec' LetrecBinder
//
// Term = AppTerm
//      | 'lambda' TypedBinders '.' Term
//      | 'if' Term 'then' Term 'else' Term
//      | 'let' LetBinder 'in' Term
//      | 'letrec' LetrecBinder 'in' Term
//
// TypedBinders = TypedBinder
//              | TypedBinder TypedBinders
//
// TypedBinder = lcid ':' Type
// 	       | '_' ':' Type
//
// Pattern = lcid
//         | '_'
//         | '{' FieldPatterns '}'
//
// FieldPatterns = FieldPattern
//               | FieldPattern ',' FieldPatterns
//
// FieldPattern = Pattern '=' lcid
//
// LetBinder = Pattern '=' Term
//
// LetrecBinder = TypedBinder '=' Term
//
// AppTerm = PathTerm
//         | AppTerm PathTerm
//         | 'succ' PathTerm
//         | 'pred' PathTerm
//         | 'iszero' PathTerm
//         | 'cons' PathTerm PathTerm
//         | 'isnil' PathTerm
//         | 'head' PathTerm
//         | 'tail' PathTerm
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

#include <array>
#include <memory>

#include "common.h"
#include "token.h"
#include "visitor.h"

// Statement.
class Stmt {
 public:
  Stmt(Location location) : location_(location) { }
  virtual ~Stmt() = default;

 private:
  Location location_;
};

class EvalStmt final : public Stmt {
 public:
  EvalStmt(Location location) : Stmt(location) { }
};

class BindTermStmt final : public Stmt {
 public:
  BindTermStmt(Location location) : Stmt(location) { }
};

class BindTypeStmt final : public Stmt {
 public:
  BindTypeStmt(Location location) : Stmt(location) { }
};

// Pattern.
class Pattern {
 public:
  Pattern(Location location) : location_(location) { }
  virtual ~Pattern() = default;

  virtual void Accept(Visitor<Pattern>* visitor) = 0;

 private:
  Location location_;
};

class VariablePattern : public Pattern {
 public:
  VariablePattern(Location location) : Pattern(location) { }

  void Accept(Visitor<Pattern>* visitor) override { }
};

class RecordPattern : public Pattern {
 public:
  RecordPattern(Location location) : Pattern(location) { }

  void Accept(Visitor<Pattern>* visitor) override { }
};

// Term.
class Term {
 public:
  Term(Location location) : location_(location) { }
  virtual ~Term() = default;

 private:
  Location location_;
};

enum class NullaryTermToken {
  True, False, Variable, Unit, Zero, Nil,
};

enum class UnaryTermToken {
  Succ, Pred, IsZero, IsNil, Head, Tail, Fix,
};

enum class BinaryTermToken {
  Cons,
};

enum class TernaryTermToken {
  If,
};

// true, false, variable, unit, zero, nil.
template<int N, typename TermToken>
class NAryTerm : public Term {
 public:
  NAryTerm(Location location) : Term(location) { }

  TermToken type() const { return type_; }

 private:
  TermToken type_;
  std::array<std::unique_ptr<Term>, N> terms_;
};

class PrimitiveTerm : public NAryTerm<0, NullaryTermToken> {
 public:
  PrimitiveTerm(Location location) : NAryTerm(location) { }
};

class UnaryTerm : public NAryTerm<1, UnaryTermToken> {
 public:
  UnaryTerm(Location location) : NAryTerm(location) { }
};

class BinaryTerm : public NAryTerm<2, BinaryTermToken> {
 public:
  BinaryTerm(Location location) : NAryTerm(location) { }
};

class TernaryTerm : public NAryTerm<3, TernaryTermToken> {
 public:
  TernaryTerm(Location location) : NAryTerm(location) { }
};
