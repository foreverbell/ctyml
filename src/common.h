#pragma once

struct Location {
  const size_t line_number;
  const size_t line_offset;

  Location(size_t number, size_t offset) : line_number(number), line_offset(offset) { }
};

template<class T>
class Visitor;

template<class T>
class VisitableBase {
 public:
  virtual void accept(Visitor<T>* visitor) = 0;
};

template<class Base, class Derived>
class Visitable : Base {
 public:
  void accept(Visitor<Base>* visitor) override {
    visitor->visit(static_cast<Derived*>(this));
  }
};
