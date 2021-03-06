#include "type-helper.h"

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "ast.h"
#include "context.h"

using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;

unique_ptr<TermType> SimplifyType(const Context* ctx, const TermType* type) {
  const UserDefinedTermType* ud_type = dynamic_cast<const UserDefinedTermType*>(type);

  if (ud_type == nullptr) {
    return nullptr;
  }
  TermTypeShifter shifter(ud_type->index() + 1);

  assert(ctx->get(ud_type->index()).second->type() != nullptr);
  auto shifted = shifter.Shift(ctx->get(ud_type->index()).second->type());
  auto deeper_shifted = SimplifyType(ctx, shifted.get());

  return deeper_shifted == nullptr ? std::move(shifted) : std::move(deeper_shifted);
}

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
  shifted_types_[type] = std::make_unique<ListTermType>(type->location(), get(type->type()).release());
}

void TermTypeShifter::Visit(const RecordTermType* type) {
  auto shifted_type = std::make_unique<RecordTermType>(type->location());
  for (size_t i = 0; i < type->size(); ++i) {
    type->get(i).second->Accept(this);
    shifted_type->add(type->get(i).first, get(type->get(i).second).release());
  }
  shifted_types_[type] = std::move(shifted_type);
}

void TermTypeShifter::Visit(const ArrowTermType* type) {
  type->type1()->Accept(this);
  type->type2()->Accept(this);
  shifted_types_[type] = std::make_unique<ArrowTermType>(
      type->location(), get(type->type1()).release(), get(type->type2()).release());
}

void TermTypeShifter::Visit(const UserDefinedTermType* type) {
  shifted_types_[type] = std::make_unique<UserDefinedTermType>(type->location(), type->index() + delta_);
}

bool ListTermTypeComparator::Compare(const ListTermType* rhs) const {
  return lhs_->type()->Compare(ctx_, rhs->type().get());
}

bool RecordTermTypeComparator::Compare(const RecordTermType* rhs) const {
  if (lhs_->size() != rhs->size()) {
    return false;
  }
  unordered_map<string, const TermType*> field_map;
  for (size_t i = 0; i < lhs_->size(); ++i) {
    field_map[lhs_->get(i).first] = lhs_->get(i).second.get();
  }
  for (size_t i = 0; i < rhs->size(); ++i) {
    auto iter = field_map.find(rhs->get(i).first);
    if (iter == field_map.end() || !iter->second->Compare(ctx_, rhs->get(i).second.get())) {
      return false;
    }
  }
  return true;
}

bool ArrowTermTypeComparator::Compare(const ArrowTermType* rhs) const {
  return lhs_->type1()->Compare(ctx_, rhs->type1().get()) && lhs_->type2()->Compare(ctx_, rhs->type2().get());
}
