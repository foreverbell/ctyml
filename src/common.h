#pragma once

struct Location {
  const ssize_t begin;
  const ssize_t end;

  Location() : begin(-1), end(-1) { }
  Location(ssize_t begin, ssize_t end) : begin(begin), end(end) { }
};

class Locatable {
 public:
  Locatable(Location location) : location_(location) { }
  Location location() const { return location_; }

 protected:
  Location location_;
};
