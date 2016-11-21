#include "pprinter.h"

#include <string>
#include <unordered_map>

#include "ast.h"
#include "context.h"

using std::string;

string PrettyPrinter::PrettyPrint(const Term* term) {
  term->Accept(this);
  string ret = get(term);
  term_pprints_.clear();
  not_nat_.clear();
  return ret;
}

string PrettyPrinter::PrettyPrint(const TermType* type) {
  type->Accept(this);
  string ret = get(type);
  type_pprints_.clear();
  return ret;
}

// Term Visitor.

void PrettyPrinter::Visit(const NullaryTerm* term) {
  switch (term->type()) {
    case NullaryTermToken::True: {
      term_pprints_[term] = "true";
    } break;
    case NullaryTermToken::False: {
      term_pprints_[term] = "false";
    } break;
    case NullaryTermToken::Unit: {
      term_pprints_[term] = "unit";
    } break;
    case NullaryTermToken::Zero: {
      term_pprints_[term] = "0";
    } break;
  }
}

void PrettyPrinter::Visit(const UnaryTerm* term) {
  term->term()->Accept(this);
  switch (term->type()) {
    case (UnaryTermToken::Succ): {
      int nat = 0;
      if (IsPrintableNatTerm(term, &nat)) {
        term_pprints_[term] = std::to_string(nat);
        break;
      }
      if (term->term()->ast_level() <= term->ast_level()) {
        term_pprints_[term] = "succ (" + get(term->term()) + ")";
      } else {
        term_pprints_[term] = "succ " + get(term->term());
      }
    } break;
    case (UnaryTermToken::Pred): {
      if (term->term()->ast_level() <= term->ast_level()) {
        term_pprints_[term] = "pred (" + get(term->term()) + ")";
      } else {
        term_pprints_[term] = "pred " + get(term->term());
      }
    } break;
    case (UnaryTermToken::IsZero): {
      if (term->term()->ast_level() <= term->ast_level()) {
        term_pprints_[term] = "iszero (" + get(term->term()) + ")";
      } else {
        term_pprints_[term] = "iszero " + get(term->term());
      }
    } break;
    case (UnaryTermToken::Head): {
      if (term->term()->ast_level() <= term->ast_level()) {
        term_pprints_[term] = "head (" + get(term->term()) + ")";
      } else {
        term_pprints_[term] = "head " + get(term->term());
      }
    } break;
    case (UnaryTermToken::Tail): {
      if (term->term()->ast_level() <= term->ast_level()) {
        term_pprints_[term] = "tail (" + get(term->term()) + ")";
      } else {
        term_pprints_[term] = "tail " + get(term->term());
      }
    } break;
    case (UnaryTermToken::IsNil): {
      if (term->term()->ast_level() <= term->ast_level()) {
        term_pprints_[term] = "isnil (" + get(term->term()) + ")";
      } else {
        term_pprints_[term] = "isnil " + get(term->term());
      }
    } break;
    case (UnaryTermToken::Fix): {
      if (term->term()->ast_level() <= term->ast_level()) {
        term_pprints_[term] = "fix (" + get(term->term()) + ")";
      } else {
        term_pprints_[term] = "fix " + get(term->term());
      }
    } break;
  }
}

void PrettyPrinter::Visit(const BinaryTerm* term) {
  term->term1()->Accept(this);
  term->term2()->Accept(this);
  switch (term->type()) {
    case BinaryTermToken::Cons: {
      // TODO(foreverbell): pretty printer for list.
      term_pprints_[term] = "cons ";
      if (term->term1()->ast_level() <= term->ast_level()) {
        term_pprints_[term] += "(" + get(term->term1()) + ")";
      } else {
        term_pprints_[term] += get(term->term1());
      }
      term_pprints_[term] += " ";
      if (term->term2()->ast_level() <= term->ast_level()) {
        term_pprints_[term] += "(" + get(term->term2()) + ")";
      } else {
        term_pprints_[term] += get(term->term2());
      }
    } break;
    case BinaryTermToken::App: {
      // Use '<' here instead of '<=', for the grammar is 'AppTerm = AppTerm PathTerm'.
      if (term->term1()->ast_level() < term->ast_level()) {
        term_pprints_[term] += "(" + get(term->term1()) + ")";
      } else {
        term_pprints_[term] += get(term->term1());
      }
      term_pprints_[term] += " ";
      if (term->term2()->ast_level() <= term->ast_level()) {
        term_pprints_[term] += "(" + get(term->term2()) + ")";
      } else {
        term_pprints_[term] += get(term->term2());
      }
    } break;
  }
}

void PrettyPrinter::Visit(const TernaryTerm* term) {
  term->term1()->Accept(this);
  term->term2()->Accept(this);
  term->term3()->Accept(this);
  switch (term->type()) {
    case (TernaryTermToken::If): {
      term_pprints_[term] = "if ";
      term_pprints_[term] += get(term->term1());
      term_pprints_[term] += " then ";
      term_pprints_[term] += get(term->term2());
      term_pprints_[term] += " else ";
      term_pprints_[term] += get(term->term3());
    } break;
  }
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
    term_pprints_[term] += term->get(i).first + ":" + get(subterm);
  }
  term_pprints_[term] += "}";
}

void PrettyPrinter::Visit(const ProjectTerm* term) {
  term->term()->Accept(this);
  if (term->term()->ast_level() < term->ast_level()) {
    term_pprints_[term] = "(" + get(term->term()) + ")." + term->field();
  } else {
    term_pprints_[term] = get(term->term()) + "." + term->field();
  }
}

void PrettyPrinter::Visit(const LetTerm* term) {
  term->bind_term()->Accept(this);

  string fresh = ctx_->PickFreshName(term->pattern()->variable());
  term->body_term()->Accept(this);
  ctx_->DropBindings(1);

  term_pprints_[term] = "let " + fresh + " = " + get(term->bind_term()) + " in " + get(term->body_term());
}

void PrettyPrinter::Visit(const AbsTerm* term) {
  string fresh = ctx_->PickFreshName(term->variable());
  term->term()->Accept(this);
  ctx_->DropBindings(1);

  term_pprints_[term] = "lambda " + term->variable() + ":" + PrettyPrint(term->variable_type()) + ". " + get(term->term());
}

void PrettyPrinter::Visit(const AscribeTerm* term) {
  term->term()->Accept(this);
  if (term->term()->ast_level() <= term->ast_level()) {
    term_pprints_[term] = "(" + get(term->term()) + ") as " + PrettyPrint(term->ascribe_type());
  } else {
    term_pprints_[term] = get(term->term()) + " as " + PrettyPrint(term->ascribe_type());
  }
}

bool PrettyPrinter::IsPrintableNatTerm(const Term* term, int* nat) {
  if (not_nat_.find(term) != not_nat_.end()) {
    return false;
  }
  const UnaryTerm* unary_term = dynamic_cast<const UnaryTerm*>(term);
  if (unary_term == nullptr || unary_term->type() != UnaryTermToken::Succ) {
    const NullaryTerm* nullary_term = dynamic_cast<const NullaryTerm*>(term);
    if (nullary_term != nullptr && nullary_term->type() == NullaryTermToken::Zero) {
      *nat = 0;
      return true;
    }
    return false;
  }
  if (IsPrintableNatTerm(unary_term->term(), nat)) {
    *nat += 1;
    return true;
  } else {
    not_nat_.insert(unary_term->term());
    return false;
  }
}

// TermType Visitor.

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
  type_pprints_[type] = "List[" + get(type->type()) + "]";
}

void PrettyPrinter::Visit(const RecordTermType* type) {
  type_pprints_[type] = "{";
  for (size_t i = 0; i < type->size(); ++i) {
    if (i != 0) {
      type_pprints_[type] += ",";
    }
    const TermType* subtype = type->get(i).second;
    subtype->Accept(this);
    type_pprints_[type] += type->get(i).first + ":" + get(subtype);
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
    type_pprints_[type] = "(" + get(type->type1()) + ")->" + get(type->type2());
  } else {
    type_pprints_[type] = get(type->type1()) + "->" + get(type->type2());
  }
}

void PrettyPrinter::Visit(const UserDefinedType* type) {
  type_pprints_[type] = ctx_->get(type->index()).first;
}
