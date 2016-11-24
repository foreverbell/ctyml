#pragma once

#include <memory>
#include <unordered_map>

#include "ast.h"
#include "visitor.h"

class Context;

class TypeChecker : public Visitor<Term> {
 public:
  TypeChecker(Context* ctx) : ctx_(ctx) { }

  std::unique_ptr<TermType> TypeCheck(const Term*);

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
  std::unique_ptr<TermType> typeof(const Term* term) {
    return std::move(typeof_[term]);
  }

  std::unordered_map<const Term*, std::unique_ptr<TermType>> typeof_;
  Context* const ctx_;
};
