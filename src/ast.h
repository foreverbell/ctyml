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
#include <vector>

#include "location.h"
#include "token.h"
#include "visitor.h"

// Statement.
class Stmt {
 public:
  Stmt(Location location) : location_(location) { }
  virtual ~Stmt() = default;

 private:
  const Location location_;
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
  const Location location_;
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

// TermType.
class TermType {

};

// Term.
class Term {
 public:
  Term(Location location) : location_(location) { }
  virtual ~Term() = default;

 private:
  const Location location_;
};

enum class NullaryTermToken {
  True, False, Variable, Unit, Zero, Nil,
};

enum class UnaryTermToken {
  Succ, Pred, IsZero, IsNil, Head, Tail, Fix,
};

enum class BinaryTermToken {
  Cons, App
};

enum class TernaryTermToken {
  If,
};

// As it is verbose to write down all n-ary terms that take n terms, a better approach is to do some abstraction,
// so we can group all n-ary terms together, through it is a little difficult to understand.
// A NAryTerm takes N terms, like (type, term_1, term_2 ... term_N).
template<int N, typename TermToken>
class NAryTerm : public Term {
 public:
  NAryTerm(Location location) : Term(location) { }

  TermToken type() const { return type_; }

 protected:
  /* const */ TermToken type_;
  std::array<std::unique_ptr<Term>, N> terms_;
};

class NullaryTerm : public NAryTerm<0, NullaryTermToken> {
 public:
  NullaryTerm(Location location) : NAryTerm(location) { }
};

class UnaryTerm : public NAryTerm<1, UnaryTermToken> {
 public:
  UnaryTerm(Location location) : NAryTerm(location) { }
  
  const Term* term() const { return terms_[0].get(); }
};

class BinaryTerm : public NAryTerm<2, BinaryTermToken> {
 public:
  BinaryTerm(Location location) : NAryTerm(location) { }

  const Term* term1() const { return terms_[0].get(); }
  const Term* term2() const { return terms_[1].get(); }
};

class TernaryTerm : public NAryTerm<3, TernaryTermToken> {
 public:
  TernaryTerm(Location location) : NAryTerm(location) { }

  const Term* term1() const { return terms_[0].get(); }
  const Term* term2() const { return terms_[1].get(); }
  const Term* term3() const { return terms_[2].get(); }
};

class RecordTerm : public Term {
 public:
  RecordTerm(Location location) : Term(location) { }

  size_t length() const { return fields_.size(); }
  std::pair<const std::string&, const Pattern*> get(int index) const {
    return std::make_pair(fields_[index].first, fields_[index].second.get());
  }

 private:
  std::vector<std::pair<std::string, std::unique_ptr<Pattern>>> fields_;
};

class ProjectTerm : public Term {
 public:
  ProjectTerm(Location location) : Term(location) { }

  const Term* term() const { return term_.get(); }
  const std::string& field() const { return field_; }

 private:
  const std::unique_ptr<Term> term_;
  const std::string field_;
};

class LetTerm : public Term {
 public:
  LetTerm(Location location) : Term(location) { }
  
  const Pattern* pattern() const { return pattern_.get(); }
  const Term* bind_term() const { return term1_.get(); }
  const Term* body_term() const { return term2_.get(); }

 private:
  const std::unique_ptr<Pattern> pattern_;
  const std::unique_ptr<Term> term1_, term2_;
};

class AbsTerm : public Term {
 public:
  AbsTerm(Location location) : Term(location) { }

  const std::string& variable() const { return variable_; }
  const TermType* variable_type() const { return variable_type_.get(); }
  const Term* term() const { return term_.get(); }

 private:
  const std::string variable_;
  const std::unique_ptr<TermType> variable_type_;
  const std::unique_ptr<Term> term_;
};

class AscribeTerm : public Term {
 public:
  AscribeTerm(Location location) : Term(location) { }

  const Term* term() const { return term_.get(); }
  const TermType* ascribe_type() const { return ascribe_type_.get(); }

 private:
  const std::unique_ptr<Term> term_;
  const std::unique_ptr<TermType> ascribe_type_;
};
