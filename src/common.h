#pragma once

struct Location {
  int line_number;
  int line_offset;

  Location() : line_number(-1), line_offset(-1) { }
  Location(int number, int offset) : line_number(number), line_offset(offset) { }
};


