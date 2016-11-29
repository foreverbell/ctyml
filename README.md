# ctyml

## Introduction

Too many lambda calculus implementations in functional programming language, I need one in C++.

TODO

## Example

```ocaml
letrec gen:Nat->List[Nat] =
  lambda x:Nat.
    if iszero x
      then nil[Nat]
      else cons x (gen (pred x));

letrec plus:Nat->Nat->Nat =
  lambda a:Nat b:Nat.
    if iszero a
      then b
      else plus (pred a) (succ b);

letrec sum:List[Nat]->Nat =
  lambda l:List[Nat].
    if isnil l
      then 0
      else plus (head l) (sum (tail l))
in sum (gen 23);

---
276
```

## Grammar

See (src/ast.h)[https://github.com/foreverbell/ctyml/blob/master/src/ast.h]

## TODO list

* Subtyping
* Polymorphism
* Full partial function support (treat `succ` and `cons` etc as a declared function in context, instead of a keyword, this depends on polymorphism)
* Hindley Milner type inference
* Evaluator is slow (avoid shifting terms without variable)

## Build

```bash
cmake .
make
./ctyml -i
```

## Test coverage

Requires coverage tool `lcov` and `genhtml`.

```bash
cmake -DCMAKE_BUILD_TYPE=Coverage .
make all ctyml_coverage
```
