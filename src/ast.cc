#include "ast.h"

#include <memory>

#include "type-helper.h"

using std::unique_ptr;

#define TermTypeCompare(Type) \
  bool Type::Compare(const Context* ctx, const TermType* rhs) const { \
    unique_ptr<TermTypeComparator> comparator(rhs->CreateComparator(ctx)); \
    return comparator->Compare(this); \
  }

TermTypeCompare(BoolTermType);
TermTypeCompare(NatTermType);
TermTypeCompare(UnitTermType);
TermTypeCompare(ListTermType);
TermTypeCompare(RecordTermType);
TermTypeCompare(ArrowTermType);

bool UserDefinedTermType::Compare(const Context* ctx, const TermType* rhs) const {
  unique_ptr<TermType> simplified = SimplifyType(ctx, this);

  assert(simplified != nullptr);
  return simplified->Compare(ctx, rhs);
}

TermTypeComparator* BoolTermType::CreateComparator(const Context*) const {
  return new BoolTermTypeComparator();
}

TermTypeComparator* NatTermType::CreateComparator(const Context*) const {
  return new NatTermTypeComparator();
}

TermTypeComparator* UnitTermType::CreateComparator(const Context*) const {
  return new UnitTermTypeComparator();
}

TermTypeComparator* ListTermType::CreateComparator(const Context* ctx) const {
  return new ListTermTypeComparator(ctx, this);
}

TermTypeComparator* RecordTermType::CreateComparator(const Context* ctx) const {
  return new RecordTermTypeComparator(ctx, this);
}

TermTypeComparator* ArrowTermType::CreateComparator(const Context* ctx) const {
  return new ArrowTermTypeComparator(ctx, this);
}

TermTypeComparator* UserDefinedTermType::CreateComparator(const Context* ctx) const {
  unique_ptr<TermType> simplifed = SimplifyType(ctx, this);

  assert(simplifed != nullptr);
  return simplifed->CreateComparator(ctx);
}
