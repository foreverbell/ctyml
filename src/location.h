#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Locatable;

struct Location {
  const ssize_t begin;
  const ssize_t end;

  Location() : begin(-1), end(-1) { }
  Location(ssize_t begin, ssize_t end) : begin(begin), end(end) { }
  Location(const Locatable* l, const Locatable* r);
};

class Locatable {
 public:
  Locatable(Location location) : location_(location) { }
  Location location() const { return location_; }

 protected:
  Location location_;
};

inline Location::Location(const Locatable* l, const Locatable* r)
  : begin(l->location().begin), end(r->location().end) { }

class Locator {
 public:
  Locator(const std::string& input);

  Locator(const Locator&) = delete;
  Locator& operator=(const Locator&) = delete;

  void Locate(Location location, int* line1, int* column1, int* line2, int* column2);

 private:
  const std::string& input_;
  std::vector<int> linemap_;
};
