#include "type-checker.h"

#include <memory>

#include "context.h"
#include "error/type-exception.h"
#include "type-helper.h"

using std::unique_ptr;

namespace {

template <typename T>
T* type_cast(const Context* ctx, TermType* ptr) {
  const unique_ptr<TermType> simplified_ptr = SimplifyType(ctx, ptr);
  return dynamic_cast<T*>(simplified_ptr == nullptr ? ptr : simplified_ptr.get());
}

}  // namespace

void TypeChecker::Visit(const NullaryTerm* term) {
  switch (term->type()) {
    case NullaryTermToken::Unit: {
      typeof_[term] = std::make_unique<UnitTermType>(term->location());
    } break;
    case NullaryTermToken::Zero: {
      typeof_[term] = std::make_unique<NatTermType>(term->location());
    } break;
    case NullaryTermToken::True:
    case NullaryTermToken::False: {
      typeof_[term] = std::make_unique<BoolTermType>(term->location());
    } break;
  }
}

void TypeChecker::Visit(const UnaryTerm* term) {
  term->term()->Accept(this);
  unique_ptr<TermType> subtype = typeof(term->term());

  switch (term->type()) {
    case UnaryTermToken::Succ:
    case UnaryTermToken::Pred:
    case UnaryTermToken::IsZero: {
      if (!type_cast<NatTermType>(ctx_, subtype.get())) {
        throw type_exception(term->location(), "<succ / pred / iszero> expects Nat type");
      }
      typeof_[term] = std::move(subtype);
    } break;
    case UnaryTermToken::Fix: {
      ArrowTermType* const arrow_type = type_cast<ArrowTermType>(ctx_, subtype.get());
      if (!arrow_type) {
        throw type_exception(term->location(), "<fix> expects Arrow type");
      }
      if (!arrow_type->type1()->Compare(ctx_, arrow_type->type2().get())) {
        throw type_exception(term->location(), "result of <fix> body is not compatible with domain");
      }
      typeof_[term] = unique_ptr<TermType>(std::move(arrow_type->type1()));
    } break;
    case UnaryTermToken::Head: {
      ListTermType* const list_type = type_cast<ListTermType>(ctx_, subtype.get());
      if (!list_type) {
        throw type_exception(term->location(), "<head> expects List type");
      }
      typeof_[term] = unique_ptr<TermType>(std::move(list_type->type()));
    } break;
    case UnaryTermToken::Tail:
    case UnaryTermToken::IsNil: {
      ListTermType* const list_type = type_cast<ListTermType>(ctx_, subtype.get());
      if (!list_type) {
        throw type_exception(term->location(), "<tail / isnil> expects List type");
      }
      typeof_[term] = std::move(subtype);
    } break;
  }
}

void TypeChecker::Visit(const BinaryTerm* term) {
  term->term1()->Accept(this);
  term->term2()->Accept(this);
  unique_ptr<TermType> subtype1 = typeof(term->term1());
  unique_ptr<TermType> subtype2 = typeof(term->term2());

  switch (term->type()) {
    case BinaryTermToken::Cons: {
      ListTermType* const list_type = type_cast<ListTermType>(ctx_, subtype2.get());
      if (!list_type) {
        throw type_exception(term->location(), "<cons> expects List type on 2nd parameter");
      }
      if (!subtype1->Compare(ctx_, list_type->type().get())) {
        throw type_exception(term->location(), "list head and tail of <cons> are incompatible");
      }
      typeof_[term] = std::move(subtype2);
    } break;
    case BinaryTermToken::App: {
      ArrowTermType* const arrow_type = type_cast<ArrowTermType>(ctx_, subtype1.get());
      if (!arrow_type) {
        throw type_exception(term->location(), "expects arrow type");
      }
      if (!arrow_type->type1()->Compare(ctx_, subtype2.get())) {
        throw type_exception(term->location(), "parameter type is mismatch");
      }
      typeof_[term] = std::move(arrow_type->type2());
    } break;
  }
}

void TypeChecker::Visit(const TernaryTerm* term) {
  term->term1()->Accept(this);
  term->term2()->Accept(this);
  term->term3()->Accept(this);
  unique_ptr<TermType> subtype1 = typeof(term->term1());
  unique_ptr<TermType> subtype2 = typeof(term->term2());
  unique_ptr<TermType> subtype3 = typeof(term->term3());

  switch (term->type()) {
    case TernaryTermToken::If: {
      BoolTermType* const bool_type = type_cast<BoolTermType>(ctx_, subtype1.get());
      if (!bool_type) {
        throw type_exception(term->location(), "guard of conditional not a boolean");
      }
      if (!subtype2->Compare(ctx_, subtype3.get())) {
        throw type_exception(term->location(), "arms of conditional have different types");
      }
      typeof_[term] = std::move(subtype2);
    } break;
  }
}

void TypeChecker::Visit(const NilTerm* term) {
  typeof_[term] = std::make_unique<ListTermType>(term->location(), term->list_type()->clone());
}

void TypeChecker::Visit(const VariableTerm* term) {
  const TermType* const type = ctx_->get(term->index()).second->type();
  assert(type != nullptr);
  TermTypeShifter shifter(term->index());
  typeof_[term] = shifter.Shift(type);
}

void TypeChecker::Visit(const RecordTerm* term) {
  unique_ptr<RecordTermType> record_type = std::make_unique<RecordTermType>(term->location());

  for (size_t i = 0; i < term->size(); ++i) {
    term->get(i).second->Accept(this);
    record_type->add(term->get(i).first, std::move(typeof(term->get(i).second)).release());
  }
  typeof_[term] = std::move(record_type);
}

void TypeChecker::Visit(const ProjectTerm* term) {
  term->term()->Accept(this);
  unique_ptr<TermType> subtype = typeof(term->term());

  RecordTermType* const record_type = type_cast<RecordTermType>(ctx_, subtype.get());
  if (!record_type) {
    throw type_exception(term->location(), "expect record type for field projection");
  }
  for (size_t i = 0; i < record_type->size(); ++i) {
    if (record_type->get(i).first == term->field()) {
      typeof_[term] = std::move(record_type->get(i).second);
      return;
    }
  }
  throw type_exception(term->location(), "field <" + term->field() + "> not found for projection");
}

void TypeChecker::Visit(const LetTerm* term) {
  term->bind_term()->Accept(this);
  ctx_->AddBinding(term->pattern()->variable(), new Binding(nullptr, typeof(term->bind_term()).get()));
  term->body_term()->Accept(this);
  ctx_->DropBindings(1);

  unique_ptr<TermType> body_type = typeof(term->body_term());
  TermTypeShifter shifter(-1);
  typeof_[term] = shifter.Shift(body_type.get());
}

void TypeChecker::Visit(const AbsTerm* term) {
  ctx_->AddBinding(term->variable(), new Binding(nullptr, term->variable_type()));
  term->term()->Accept(this);
  ctx_->DropBindings(1);

  unique_ptr<TermType> subtype = typeof(term->term());
  TermTypeShifter shifter(-1);
  typeof_[term] = shifter.Shift(subtype.get());
}

void TypeChecker::Visit(const AscribeTerm* term) {
  term->term()->Accept(this);
  unique_ptr<TermType> subtype = typeof(term->term());

  if (!subtype->Compare(ctx_, term->ascribe_type())) {
    throw type_exception(term->location(), "body of as-term does not have the expected type");
  }
  typeof_[term] = std::move(subtype);
}