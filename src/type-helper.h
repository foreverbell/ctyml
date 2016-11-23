#pragma once

#include <memory>
#include <unordered_map>

#include "visitor.h"

class Context;

class TermType;
class BoolTermType;
class NatTermType;
class UnitTermType;
class ListTermType;
class RecordTermType;
class ArrowTermType;
class UserDefinedTermType;

// TermType shifter.
class TermTypeShifter : public Visitor<TermType> {
 public:
  TermTypeShifter(int delta, const Context* ctx) : delta_(delta), ctx_(ctx) { }

  std::unique_ptr<TermType> Shift(const TermType*);

  void Visit(const BoolTermType*) override;
  void Visit(const NatTermType*) override;
  void Visit(const UnitTermType*) override;
  void Visit(const ListTermType*) override;
  void Visit(const RecordTermType*) override;
  void Visit(const ArrowTermType*) override;
  void Visit(const UserDefinedTermType*) override;

 private:
  const int delta_;
  const Context* const ctx_;
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

  static TermTypeComparator* CreateUserDefinedTypeComparator(const Context* ctx, const UserDefinedTermType* type);
};

class BoolTermTypeComparator : public TermTypeComparator {
 public:
  bool Compare(const BoolTermType*) const override { return true; }
};

class NatTermTypeComparator : public TermTypeComparator {
 public:
  bool Compare(const NatTermType*) const override { return true; }
};

class UnitTermTypeComparator : public TermTypeComparator {
 public:
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
