#include "pprinter.h"

#include <string>
#include <unordered_map>

#include "ast.h"
#include "context.h"

void PrettyPrinter::Visit(const BoolTermType* type) {
  pprints_[type] = "Bool";
}

void PrettyPrinter::Visit(const NatTermType* type) {
  pprints_[type] = "Nat";
}

void PrettyPrinter::Visit(const UnitTermType* type) {
  pprints_[type] = "Unit";
}

void PrettyPrinter::Visit(const ListTermType* type) {
  type->type()->Accept(this);
  pprints_[type] = "List[" + pprints_[type->type()] + "]";
}

void PrettyPrinter::Visit(const RecordTermType* type) {
  pprints_[type] = "{";
  for (size_t i = 0; i < type->size(); ++i) {
    if (i != 0) {
      pprints_[type] += ",";
    }
    const TermType* subtype = type->get(i).second;
    subtype->Accept(this);
    pprints_[type] += type->get(i).first + ":" + pprints_[subtype];
  }
  pprints_[type] = "}";
}

void PrettyPrinter::Visit(const ArrowTermType* type) {
  type->type1()->Accept(this);
  type->type2()->Accept(this);

  if (type->type1()->ast_level() <= type->ast_level()) {
    pprints_[type] = "(" + pprints_[type->type1()] + ")->" + pprints_[type->type2()];
  } else {
    pprints_[type] = pprints_[type->type1()] + "->" + pprints_[type->type2()];
  }
}

void PrettyPrinter::Visit(const UserDefinedType* type) {
  pprints_[type] = ctx_->get(type->index()).first;
}
