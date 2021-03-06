#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "ast.h"
#include "visitor.h"

class Context;

class TermMapper : public Visitor<Term> {
 public:
  TermVisitorOverrides;

  virtual std::unique_ptr<Term> VariableMap(Location location, int var) = 0;

 protected:
  std::unique_ptr<Term> Map(const Term*);
  int depth() const { return depth_; }

 private:
  std::unique_ptr<Term> get(const Term* term) { return std::move(result_[term]); }
  std::unique_ptr<Term> get(const std::unique_ptr<Term>& term) { return std::move(result_[term.get()]); }

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

// Evaluate term into a normal value. Valid values are:
// * true, false
// * zero, succ zero, succ (succ zero), ...
// * nil, cons v nil, cons v_1 (cons v_2 nil), ...
// * unit
// * {f_1: v_1, f_2: v_2, ...}
// * lambda x. t
class TermEvaluator : public Visitor<Term> {
 public:
  TermEvaluator(Context* ctx) : ctx_(ctx) { }
  TermVisitorOverrides;

  std::unique_ptr<Term> Evaluate(const Term*);

 private:
  std::unique_ptr<Term> Substitute(const Term* term, const Term* to);

  std::unique_ptr<Term> eval(const Term* term) { return std::move(result_[term]); }
  std::unique_ptr<Term> eval(const std::unique_ptr<Term>& term) { return std::move(result_[term.get()]); }

  std::unordered_map<const Term*, std::unique_ptr<Term>> result_;
  Context* const ctx_;
};
