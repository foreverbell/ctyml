#pragma once

#include <memory>
#include <unordered_map>

#include "ast.h"
#include "visitor.h"

class Context;

class TypeChecker : public Visitor<Term> {
 public:
  TypeChecker(Context* ctx) : ctx_(ctx) { }
  TermVisitorOverrides;

  std::unique_ptr<TermType> TypeCheck(const Term*);

 private:
  std::unique_ptr<TermType> typeof(const Term* term) { return std::move(typeof_[term]); }
  std::unique_ptr<TermType> typeof(const std::unique_ptr<Term>& term) { return std::move(typeof_[term.get()]); }

  std::unordered_map<const Term*, std::unique_ptr<TermType>> typeof_;
  Context* const ctx_;
};
