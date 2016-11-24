#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Locatable;

struct Location {
  size_t begin;
  size_t end;

  Location(size_t begin, size_t end) : begin(begin), end(end) { }
  Location(Location l, Location r) : begin(l.begin), end(r.end) { }
  Location(const Locatable* l, const Locatable* r);
};

class Locatable {
 public:
  Locatable(Location location) : location_(location) { }
  Location location() const { return location_; }
  void relocate(Location location) { location_ = location; }

 protected:
  Location location_;
};

inline Location::Location(const Locatable* l, const Locatable* r)
  : begin(l->location().begin), end(r->location().end) { }

class Locator {
 public:
  Locator(const std::string& filename, const std::string& input);

  Locator(const Locator&) = delete;
  Locator& operator=(const Locator&) = delete;

  void Locate(Location location, int* line1, int* column1, int* line2, int* column2) const;

  void PrintLocation(int fd, Location location) const;
  void Error(int fd, Location location, const std::string& error) const;

 private:
  const std::string filename_;
  const std::string& input_;
  std::vector<size_t> linemap_;
};
