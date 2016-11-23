#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "ast.h"
#include "visitor.h"

class Context;

class PrettyPrinter : public Visitor<Term>, public Visitor<TermType> {
 public:
  PrettyPrinter(Context* ctx) : ctx_(ctx) { }

  // Term visitors.
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

  // Type visitors.
  void Visit(const BoolTermType*) override;
  void Visit(const NatTermType*) override;
  void Visit(const UnitTermType*) override;
  void Visit(const ListTermType*) override;
  void Visit(const RecordTermType*) override;
  void Visit(const ArrowTermType*) override;
  void Visit(const UserDefinedTermType*) override;

  std::string PrettyPrint(const Term* term);
  std::string PrettyPrint(const TermType* type);

 private:
  std::string get(const TermType* type) {
    return std::move(type_pprints_[type]);
  }

  std::string get(const Term* term) {
    return std::move(term_pprints_[term]);
  }

  bool IsPrintableNatTerm(const Term* term, int* nat);

  std::unordered_map<const TermType*, std::string> type_pprints_;
  std::unordered_map<const Term*, std::string> term_pprints_;
  Context* const ctx_;

  // If a 'succ' term is contained in this unordered_set, it is not a printable Nat,
  // A printable Nat is a list of 'succ's, i.e. succ (succ (succ ... 0))).
  std::unordered_set<const Term*> not_nat_;
};
