#pragma once

template<class T>
class Visitor;

template<class Base>
class Visitable {
 public:
  virtual void Accept(Visitor<Base>* visitor) const = 0;
};

// Virtual inheritance is required to solve the diamond inheritance issue.
//
//            Visitable
//              / \
//             /   \
//            /     \
// VisitableImpl   Abstract Class
//            \     /
//             \   /
//              \ /
//         Concrete Class
//
// Both VisitableImpl and Abstract class will inherit Visitable::Accept, but the static polymorphism implemenation of
// Visitable::Accept lays in VisitableImpl. VisitableImpl and Abstact Class both need to inherit Visitable virtually.

template<class Base, class Derived>
class VisitableImpl : public virtual Visitable<Base> {
 public:
  void Accept(Visitor<Base>* visitor) const override {
    visitor->Visit(static_cast<const Derived*>(this));
  }
};

// TermType visitor.
class TermType;
class BoolTermType;
class NatTermType;
class UnitTermType;
class ListTermType;
class RecordTermType;
class ArrowTermType;
class UserDefinedType;

template<>
class Visitor<TermType> {
 public:
  virtual void Visit(const BoolTermType*) = 0;
  virtual void Visit(const NatTermType*) = 0;
  virtual void Visit(const UnitTermType*) = 0;
  virtual void Visit(const ListTermType*) = 0;
  virtual void Visit(const RecordTermType*) = 0;
  virtual void Visit(const ArrowTermType*) = 0;
  virtual void Visit(const UserDefinedType*) = 0;
};
