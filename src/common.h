#pragma once

struct Location {
  int line_number;
  int line_offset;

  Location() : line_number(-1), line_offset(-1) { }
  Location(int number, int offset) : line_number(number), line_offset(offset) { }
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
