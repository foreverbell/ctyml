#pragma once

#include <memory>
#include <unordered_map>

#include "visitor.h"

class Context;

// Recusively simplify a type in the given context <ctx>, until the outer-most type is not user-defined.
// Returns nullptr if no simplification can be done.
std::unique_ptr<TermType> SimplifyType(const Context* ctx, const TermType* type);

// TermType shifter.
class TermTypeShifter : public Visitor<TermType> {
 public:
  TermTypeShifter(int delta) : delta_(delta) { }
  TermTypeVisitorOverrides;

  std::unique_ptr<TermType> Shift(const TermType*);

 private:
  std::unique_ptr<TermType> get(const TermType* type) { return std::move(shifted_types_[type]); }
  std::unique_ptr<TermType> get(const std::unique_ptr<TermType>& type) { return std::move(shifted_types_[type.get()]); }

  const int delta_;
  std::unordered_map<const TermType*, std::unique_ptr<TermType>> shifted_types_;
};

// TermType comparator.

class TermTypeComparator {
 public:
  virtual ~TermTypeComparator() = default;

  virtual bool Compare(const BoolTermType*) const { return false; }
  virtual bool Compare(const NatTermType*) const { return false; }
  virtual bool Compare(const UnitTermType*) const { return false; }
  virtual bool Compare(const ListTermType*) const { return false; }
  virtual bool Compare(const RecordTermType*) const { return false; }
  virtual bool Compare(const ArrowTermType*) const { return false; }
};

class BoolTermTypeComparator : public TermTypeComparator {
 public:
  BoolTermTypeComparator(const Context*, const BoolTermType*) { }
  bool Compare(const BoolTermType*) const override { return true; }
};

class NatTermTypeComparator : public TermTypeComparator {
 public:
  NatTermTypeComparator(const Context*, const NatTermType*) { }
  bool Compare(const NatTermType*) const override { return true; }
};

class UnitTermTypeComparator : public TermTypeComparator {
 public:
  UnitTermTypeComparator(const Context*, const UnitTermType*) { }
  bool Compare(const UnitTermType*) const override { return true; }
};

class ListTermTypeComparator : public TermTypeComparator {
 public:
  ListTermTypeComparator(const Context* ctx, const ListTermType* lhs) : ctx_(ctx), lhs_(lhs) { }
  bool Compare(const ListTermType* rhs) const override;

 private:
  const Context* const ctx_;
  const ListTermType* const lhs_;
};

class RecordTermTypeComparator : public TermTypeComparator {
 public:
  RecordTermTypeComparator(const Context* ctx, const RecordTermType* lhs) : ctx_(ctx), lhs_(lhs) { }
  bool Compare(const RecordTermType* rhs) const override;

 private:
  const Context* const ctx_;
  const RecordTermType* const lhs_;
};

class ArrowTermTypeComparator : public TermTypeComparator {
 public:
  ArrowTermTypeComparator(const Context* ctx, const ArrowTermType* lhs) : ctx_(ctx), lhs_(lhs) { }
  bool Compare(const ArrowTermType* rhs) const override;

 private:
  const Context* const ctx_;
  const ArrowTermType* const lhs_;
};
