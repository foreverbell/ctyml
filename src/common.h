#pragma once

struct Location {
  const size_t line_number;
  const size_t line_offset;

  Location(size_t number, size_t offset) : line_number(number), line_offset(offset) { }
};
