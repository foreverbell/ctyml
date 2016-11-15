#pragma once

template<class T>
class Visitor;

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
