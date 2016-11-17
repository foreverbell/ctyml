#include "pprinter.h"

#include <string>
#include <unordered_map>

#include "ast.h"

void PrettyPrinter::Visit(BoolTermType* term) {
  pprints_[term] = "Bool";
}

void PrettyPrinter::Visit(NatTermType*) {
}

void PrettyPrinter::Visit(UnitTermType*) {
}

void PrettyPrinter::Visit(ListTermType*) {
}

void PrettyPrinter::Visit(RecordTermType*) {
}

void PrettyPrinter::Visit(ArrowTermType*) {
}

void PrettyPrinter::Visit(UserDefinedType*) {
}
