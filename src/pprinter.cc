#include "pprinter.h"

#include <string>
#include <unordered_map>

#include "ast.h"
#include "context.h"

using std::string;

string PrettyPrinter::PrettyPrint(const Term* term) {
  term->Accept(this);
  string ret = std::move(term_pprints_[term]);
  term_pprints_.clear();
  not_nat_.clear();
  return ret;
}

string PrettyPrinter::PrettyPrint(const TermType* type) {
  type->Accept(this);
  string ret = std::move(type_pprints_[type]);
  type_pprints_.clear();
  return ret;
}

void PrettyPrinter::Visit(const NullaryTerm* term) {
  switch (term->type()) {
    case NullaryTermToken::True:
      term_pprints_[term] = "true";
      break;
    case NullaryTermToken::False:
      term_pprints_[term] = "false";
      break;
    case NullaryTermToken::Unit:
      term_pprints_[term] = "unit";
      break;
    case NullaryTermToken::Zero:
      term_pprints_[term] = "0";
      break;
  }
}

void PrettyPrinter::Visit(const UnaryTerm* term) {

}

void PrettyPrinter::Visit(const BinaryTerm* term) {

}

void PrettyPrinter::Visit(const TernaryTerm* term) {

}

void PrettyPrinter::Visit(const NilTerm* term) {
  term_pprints_[term] = "nil[" + PrettyPrint(term->list_type()) + "]";
}

void PrettyPrinter::Visit(const VariableTerm* term) {
  term_pprints_[term] = ctx_->get(term->index()).first;
}

void PrettyPrinter::Visit(const RecordTerm* term) {
  term_pprints_[term] = "{";
  for (size_t i = 0; i < term->size(); ++i) {
    if (i != 0) {
      term_pprints_[term] += ",";
    }
    const Term* subterm = term->get(i).second;
    subterm->Accept(this);
    term_pprints_[term] += term->get(i).first + ":" + term_pprints_[subterm];
  }
  term_pprints_[term] += "}";

}

void PrettyPrinter::Visit(const ProjectTerm* term) {

}

void PrettyPrinter::Visit(const LetTerm* term) {

}

void PrettyPrinter::Visit(const AbsTerm* term) {

}

void PrettyPrinter::Visit(const AscribeTerm* term) {

}

void PrettyPrinter::Visit(const BoolTermType* type) {
  type_pprints_[type] = "Bool";
}

void PrettyPrinter::Visit(const NatTermType* type) {
  type_pprints_[type] = "Nat";
}

void PrettyPrinter::Visit(const UnitTermType* type) {
  type_pprints_[type] = "Unit";
}

void PrettyPrinter::Visit(const ListTermType* type) {
  type->type()->Accept(this);
  type_pprints_[type] = "List[" + type_pprints_[type->type()] + "]";
}

void PrettyPrinter::Visit(const RecordTermType* type) {
  type_pprints_[type] = "{";
  for (size_t i = 0; i < type->size(); ++i) {
    if (i != 0) {
      type_pprints_[type] += ",";
    }
    const TermType* subtype = type->get(i).second;
    subtype->Accept(this);
    type_pprints_[type] += type->get(i).first + ":" + type_pprints_[subtype];
  }
  type_pprints_[type] += "}";
}

void PrettyPrinter::Visit(const ArrowTermType* type) {
  type->type1()->Accept(this);
  type->type2()->Accept(this);

  // Add surrounding parens if the lhs of an arrow is a AtomicType (ast_level <= 1).
  // Grammar context:
  //   Type = ArrowType;
  //   ArrowType = AtomicType '->' ArrowType;
  //   AtomicType = '(' Type ')'.
  if (type->type1()->ast_level() <= type->ast_level()) {
    type_pprints_[type] = "(" + type_pprints_[type->type1()] + ")->" + type_pprints_[type->type2()];
  } else {
    type_pprints_[type] = type_pprints_[type->type1()] + "->" + type_pprints_[type->type2()];
  }
}

void PrettyPrinter::Visit(const UserDefinedType* type) {
  type_pprints_[type] = ctx_->get(type->index()).first;
}
