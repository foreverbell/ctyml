#pragma once

#include <string>
#include <unordered_map>

#include "ast.h"
#include "visitor.h"

class Context;

class PrettyPrinter : public Visitor<TermType> {
 public:
  void Visit(const BoolTermType*) override;
  void Visit(const NatTermType*) override;
  void Visit(const UnitTermType*) override;
  void Visit(const ListTermType*) override;
  void Visit(const RecordTermType*) override;
  void Visit(const ArrowTermType*) override;
  void Visit(const UserDefinedType*) override;

  std::string PrettyPrint(const TermType* type);

 private:
  std::unordered_map<const TermType*, std::string> pprints_;
  const Context* ctx_;
};
