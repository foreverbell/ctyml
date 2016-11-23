#include "type-helper.h"

#include <cassert>
#include <memory>
#include <vector>

#include "ast.h"
#include "context.h"

using std::unique_ptr;
using std::vector;

unique_ptr<TermType> TermTypeShifter::Shift(const TermType* type) {
  type->Accept(this);
  unique_ptr<TermType> ret = std::move(shifted_types_[type]);
  shifted_types_.clear();
  return ret;
}

void TermTypeShifter::Visit(const BoolTermType* type) {
  shifted_types_[type] = std::make_unique<BoolTermType>(type->location());
}

void TermTypeShifter::Visit(const NatTermType* type) {
  shifted_types_[type] = std::make_unique<NatTermType>(type->location());
}

void TermTypeShifter::Visit(const UnitTermType* type) {
  shifted_types_[type] = std::make_unique<UnitTermType>(type->location());
}

void TermTypeShifter::Visit(const ListTermType* type) {
  type->type()->Accept(this);
  shifted_types_[type] = std::make_unique<ListTermType>(type->location(), shifted_types_[type->type()].release());
}

void TermTypeShifter::Visit(const RecordTermType* type) {
  auto shifted_type = std::make_unique<RecordTermType>(type->location());
  for (size_t i = 0; i < type->size(); ++i) {
    type->get(i).second->Accept(this);
    shifted_type->add(type->get(i).first, shifted_types_[type->get(i).second].release());
  }
  shifted_types_[type] = std::move(shifted_type);
}

void TermTypeShifter::Visit(const ArrowTermType* type) {
  type->type1()->Accept(this);
  type->type2()->Accept(this);
  shifted_types_[type] = std::make_unique<ArrowTermType>(
      type->location(), shifted_types_[type->type1()].release(), shifted_types_[type->type2()].release());
}

void TermTypeShifter::Visit(const UserDefinedTermType* type) {
  shifted_types_[type] = std::make_unique<UserDefinedTermType>(type->location(), type->index() + delta_);
}

namespace {

// Recusively simplify a type in the given context <ctx>, until the outer-most type is not user-defined.
// Returns nullptr if no simplification can be done.
unique_ptr<TermType> SimplifyType(const Context* ctx, const TermType* type) {
  const UserDefinedTermType* ud_type = dynamic_cast<const UserDefinedTermType*>(type);

  if (ud_type == nullptr) {
    return nullptr;
  }
  TermTypeShifter shifter(ud_type->index() + 1, ctx);

  assert(ctx->get(ud_type->index()).second->type() != nullptr);
  auto shifted = shifter.Shift(ctx->get(ud_type->index()).second->type());
  auto deeper_shifted = SimplifyType(ctx, shifted.get());

  return std::move(deeper_shifted == nullptr ? shifted : deeper_shifted);
}

}  // namespace

// TODO(foreverbell): Implementation.
bool ListTermTypeComparator::Compare(const ListTermType* rhs) const {
  return false;
}

bool RecordTermTypeComparator::Compare(const RecordTermType* rhs) const {
  if (lhs_->size() != rhs->size()) {
    return false;
  }
  return false;
}

bool ArrowTermTypeComparator::Compare(const ArrowTermType* rhs) const {
  return false;
}
