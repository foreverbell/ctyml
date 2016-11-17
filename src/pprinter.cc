#include "pprinter.h"

#include <string>
#include <unordered_map>

#include "ast.h"
#include "context.h"

void PrettyPrinter::Visit(BoolTermType* type) {
  pprints_[type] = "Bool";
}

void PrettyPrinter::Visit(NatTermType* type) {
  pprints_[type] = "Nat";
}

void PrettyPrinter::Visit(UnitTermType* type) {
  pprints_[type] = "Unit";
}

void PrettyPrinter::Visit(ListTermType* type) {
  type->type()->Accept(this);
  pprints_[type] = "List[" + pprints_[type->type()] + "]";
}

void PrettyPrinter::Visit(RecordTermType* type) {
  pprints_[type] = "{";
  for (size_t i = 0; i < type->size(); ++i) {
    if (i != 0) {
      pprints_[type] += ",";
    }
    TermType* subtype = type->get(i).second;
    subtype->Accept(this);
    pprints_[type] += type->get(i).first + ":" + pprints_[subtype];
  }
  pprints_[type] = "}";
}

void PrettyPrinter::Visit(ArrowTermType* type) {
  type->type1()->Accept(this);
  type->type2()->Accept(this);

  if (type->type1()->ast_level() <= type->ast_level()) {
    pprints_[type] = "(" + pprints_[type->type1()] + ")->" + pprints_[type->type2()];
  } else {
    pprints_[type] = pprints_[type->type1()] + "->" + pprints_[type->type2()];
  }
}

void PrettyPrinter::Visit(UserDefinedType* type) {
  pprints_[type] = ctx_->get(type->index()).first;
}
