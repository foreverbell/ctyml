#pragma once

template<class T>
class Visitor;

template<class Base>
class Visitable {
 public:
  virtual void Accept(Visitor<Base>* visitor) = 0;
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
  void Accept(Visitor<Base>* visitor) override {
    visitor->Visit(static_cast<Derived*>(this));
  }
};

// Pattern visitor.
class Pattern;
class VariablePattern;
class RecordPattern;

template<>
class Visitor<Pattern> {
 public:
  virtual void Visit(VariablePattern*) = 0;
  virtual void Visit(RecordPattern*) = 0;
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
  virtual void Visit(BoolTermType*) = 0;
  virtual void Visit(NatTermType*) = 0;
  virtual void Visit(UnitTermType*) = 0;
  virtual void Visit(ListTermType*) = 0;
  virtual void Visit(RecordTermType*) = 0;
  virtual void Visit(ArrowTermType*) = 0;
  virtual void Visit(UserDefinedType*) = 0;
};
