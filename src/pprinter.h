#pragma once

#include <string>
#include <unordered_map>

#include "ast.h"
#include "visitor.h"

class Context;

class PrettyPrinter : public Visitor<TermType> {
 public:
  void Visit(BoolTermType*) override;
  void Visit(NatTermType*) override;
  void Visit(UnitTermType*) override;
  void Visit(ListTermType*) override;
  void Visit(RecordTermType*) override;
  void Visit(ArrowTermType*) override;
  void Visit(UserDefinedType*) override;

 private:
  std::unordered_map<const TermType*, std::string> pprints_;
  const Context* ctx_;
};
