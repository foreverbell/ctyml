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
  TermVisitorOverrides;
  TermTypeVisitorOverrides;

  std::string PrettyPrint(const Term* term);
  std::string PrettyPrint(const TermType* type);

 private:
  std::string get(const TermType* type) { return std::move(type_pprints_[type]); }
  std::string get(const std::unique_ptr<TermType>& type) { return std::move(type_pprints_[type.get()]); }

  std::string get(const Term* term) { return std::move(term_pprints_[term]); }
  std::string get(const std::unique_ptr<Term>& term) { return std::move(term_pprints_[term.get()]); }

  bool IsPrintableNatTerm(const Term* term, int* nat);

  std::unordered_map<const TermType*, std::string> type_pprints_;
  std::unordered_map<const Term*, std::string> term_pprints_;
  Context* const ctx_;

  // If a 'succ' term is contained in this unordered_set, it is not a printable Nat,
  // A printable Nat is a list of 'succ's, i.e. succ (succ (succ ... 0))).
  std::unordered_set<const Term*> not_nat_;
};
