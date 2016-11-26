#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "ast.h"
#include "visitor.h"

class Context;

class TermMapper : public Visitor<Term> {
 public:
  void Visit(const NullaryTerm*) override;
  void Visit(const UnaryTerm*) override;
  void Visit(const BinaryTerm*) override;
  void Visit(const TernaryTerm*) override;
  void Visit(const NilTerm*) override;
  void Visit(const VariableTerm*) override;
  void Visit(const RecordTerm*) override;
  void Visit(const ProjectTerm*) override;
  void Visit(const LetTerm*) override;
  void Visit(const AbsTerm*) override;
  void Visit(const AscribeTerm*) override;

  virtual std::unique_ptr<Term> VariableMap(Location location, int var) = 0;

 protected:
  std::unique_ptr<Term> Map(const Term*);
  int depth() const { return depth_; }

 private:
  std::unique_ptr<Term> get(const Term* term) {
    return std::move(result_[term]);
  }

  std::unordered_map<const Term*, std::unique_ptr<Term>> result_;
  int depth_ = 0;
};

// Shifts up deBruijn indices of all free variables by <delta>.
class TermShifter : public TermMapper {
 public:
  TermShifter(int delta) : delta_(delta) { }
  std::unique_ptr<Term> TermShift(const Term* term) { return Map(term); }

 protected:
  std::unique_ptr<Term> VariableMap(Location location, int var) override;

 private:
  const int delta_;
};

// Substitutes the variable with 0 deBruijn index.
class TermSubstituter : public TermMapper {
 public:
  TermSubstituter(const Term* substitute_to) : substitute_to_(substitute_to) { }
  std::unique_ptr<Term> TermSubstitute(const Term* term) { return Map(term); }

 protected:
  std::unique_ptr<Term> VariableMap(Location location, int var) override;

 private:
  const Term* const substitute_to_;
};

class TermEvaluator : public Visitor<Term> {
 public:
  TermEvaluator(Context* ctx) : ctx_(ctx) { }

  std::unique_ptr<Term> Evaluate(const Term*);

  void Visit(const NullaryTerm*) override;
  void Visit(const UnaryTerm*) override;
  void Visit(const BinaryTerm*) override;
  void Visit(const TernaryTerm*) override;
  void Visit(const NilTerm*) override;
  void Visit(const VariableTerm*) override;
  void Visit(const RecordTerm*) override;
  void Visit(const ProjectTerm*) override;
  void Visit(const LetTerm*) override;
  void Visit(const AbsTerm*) override;
  void Visit(const AscribeTerm*) override;

 private:
  std::unique_ptr<Term> eval(const Term* term) {
    return std::move(result_[term]);
  }

  std::unordered_map<const Term*, std::unique_ptr<Term>> result_;
  Context* const ctx_;
};
