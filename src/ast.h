#pragma once

// Context free grammar for our toy PL.
//
// Topmost = Statement
//         | Statement Topmost
//
// Statement = Term ';'
//           | 'type' ucid '=' Type ';'
//           | 'let' Pattern '=' Term ';'
//           | 'letrec' TypedBinder '=' Term ';'
//
// Term = AppTerm
//      | 'lambda' TypedBinders '.' Term
//      | 'if' Term 'then' Term 'else' Term
//      | 'let' Pattern '=' Term 'in' Term
//      | 'letrec' TypedBinder '=' Term 'in' Term
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

#include "error.h"
#include "location.h"
#include "token.h"
#include "visitor.h"

class Stmt;
class Pattern;
class Term;
class TermType;

// Statement.
class Stmt : public Locatable {
 public:
  Stmt(Location location) : Locatable(location) { }
  virtual ~Stmt() = default;
};

class EvalStmt final : public Stmt {
 public:
  EvalStmt(Location location, Term* term)
    : Stmt(location), term_(term) { }
 private:
  std::unique_ptr<Term> term_;
};

class BindTermStmt final : public Stmt {
 public:
  BindTermStmt(Location location, Pattern* pattern, Term* term)
    : Stmt(location), pattern_(pattern), term_(term) { }
 private:
  std::unique_ptr<Pattern> pattern_;
  std::unique_ptr<Term> term_;
};

class BindTypeStmt final : public Stmt {
 public:
  BindTypeStmt(Location location, const std::string& type_alias, TermType* type)
    : Stmt(location), type_alias_(type_alias), type_(type) { }
 private:
  std::string type_alias_;
  std::unique_ptr<TermType> type_;
};

// Pattern.
class Pattern : public Locatable {
 public:
  Pattern(Location location) : Locatable(location) { }
  virtual ~Pattern() = default;

  virtual void Accept(Visitor<Pattern>* visitor) = 0;
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

// TypedBinders.
class TypedBinders : public Locatable {
 public:
  TypedBinders(Location location) : Locatable(location) { }

  void add(const std::string& variable, TermType* type) {
    binders_.emplace_back(variable, std::unique_ptr<TermType>(type));
  }
  size_t size() const { return binders_.size(); }
  std::pair<std::string, std::unique_ptr<TermType>>& get(int index) {
    return binders_.at(index);
  }

 private:
  std::vector<std::pair<std::string, std::unique_ptr<TermType>>> binders_;
};

// TermType.
class TermType : public Locatable {
 public:
  TermType(Location location) : Locatable(location) { }
  virtual ~TermType() = default;
};

// Term.
class Term : public Locatable {
 public:
  Term(Location location) : Locatable(location) { }
  virtual ~Term() = default;
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
  NAryTerm(Location location, TermToken type)
    : Term(location), type_(type) { }

  TermToken type() const { return type_; }

 protected:
  const TermToken type_;
  std::array<std::unique_ptr<Term>, N> terms_;
};

class NullaryTerm : public NAryTerm<0, NullaryTermToken> {
 public:
  NullaryTerm(Location location, NullaryTermToken type)
    : NAryTerm(location, type) { }
};

class UnaryTerm : public NAryTerm<1, UnaryTermToken> {
 public:
  UnaryTerm(Location location, UnaryTermToken type, Term* term1)
    : NAryTerm(location, type) {
    terms_[0].reset(term1);
  }

  const Term* term() const { return terms_[0].get(); }
};

class BinaryTerm : public NAryTerm<2, BinaryTermToken> {
 public:
  BinaryTerm(Location location, BinaryTermToken type, Term* term1, Term* term2)
    : NAryTerm(location, type) {
    terms_[0].reset(term1);
    terms_[1].reset(term2);
  }

  const Term* term1() const { return terms_[0].get(); }
  const Term* term2() const { return terms_[1].get(); }
};

class TernaryTerm : public NAryTerm<3, TernaryTermToken> {
 public:
  TernaryTerm(Location location, TernaryTermToken type, Term* term1, Term* term2, Term* term3)
    : NAryTerm(location, type) {
    terms_[0].reset(term1);
    terms_[1].reset(term2);
    terms_[2].reset(term3);
  }

  const Term* term1() const { return terms_[0].get(); }
  const Term* term2() const { return terms_[1].get(); }
  const Term* term3() const { return terms_[2].get(); }
};

class RecordTerm : public Term {
 public:
  RecordTerm(Location location) : Term(location) { }

  size_t size() const { return fields_.size(); }
  std::pair<const std::string&, const Pattern*> get(int index) const {
    return std::make_pair(fields_.at(index).first, fields_.at(index).second.get());
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
  LetTerm(Location location, Pattern* pattern, Term* bind_term, Term* body_term)
    : Term(location), pattern_(pattern), term1_(bind_term), term2_(body_term) { }

  const Pattern* pattern() const { return pattern_.get(); }
  const Term* bind_term() const { return term1_.get(); }
  const Term* body_term() const { return term2_.get(); }

 private:
  const std::unique_ptr<Pattern> pattern_;
  const std::unique_ptr<Term> term1_, term2_;
};

class AbsTerm : public Term {
 public:
  AbsTerm(Location location, const std::string& variable, TermType* type, Term* term)
    : Term(location), variable_(variable), variable_type_(type), term_(term) { }

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
