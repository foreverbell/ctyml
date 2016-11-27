#include "ast.h"

#include <memory>

#include "type-helper.h"

using std::unique_ptr;

#define TermTypeCompare(Type, Comparator) \
  bool Type::Compare(const Context* ctx, const TermType* rhs_old) const { \
    unique_ptr<TermType> simplified = SimplifyType(ctx, rhs_old); \
    const TermType* rhs = simplified == nullptr ? rhs_old : simplified.get(); \
    unique_ptr<TermTypeComparator> comparator(rhs->CreateComparator(ctx)); \
    return comparator->Compare(this); \
  } \
   \
  TermTypeComparator* Type::CreateComparator(const Context* ctx) const { \
    return new Comparator(ctx, this); \
  }

TermTypeCompare(BoolTermType, BoolTermTypeComparator);
TermTypeCompare(NatTermType, NatTermTypeComparator);
TermTypeCompare(UnitTermType, UnitTermTypeComparator);
TermTypeCompare(ListTermType, ListTermTypeComparator);
TermTypeCompare(RecordTermType, RecordTermTypeComparator);
TermTypeCompare(ArrowTermType, ArrowTermTypeComparator);

bool UserDefinedTermType::Compare(const Context* ctx, const TermType* rhs) const {
  unique_ptr<TermType> simplified = SimplifyType(ctx, this);
  assert(simplified != nullptr);
  return simplified->Compare(ctx, rhs);
}

TermTypeComparator* UserDefinedTermType::CreateComparator(const Context* ctx) const {
  // This function will never get executed, ensured by simplification in XXTermType::Compare.
  return nullptr;
}
