#pragma once

struct Location {
  int line_number;
  int line_offset;

  Location() : line_number(-1), line_offset(-1) { }
  Location(int number, int offset) : line_number(number), line_offset(offset) { }
};

enum class TokenType {
  Undefined,
  Int,               // With a natural integer number.
  LCaseId, UCaseId,  // With an alphabet-leading identifier.
  If, Then, Else,
  Succ, Pred, IsZero,
  True, False, Zero,
  Nil, Cons,
  IsNil, Head, Tail,
  Unit,
  Bool, Nat, List, UUnit,
  Lambda, Let, In, Letrec, TypeAlias, As,
  LParen, RParen,
  LCurly, RCurly,
  LBracket, RBracket,
  Arrow, Dot, Comma, Colon, Semi, Eq, UScore,
};
