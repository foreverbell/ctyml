#include "location.h"

#include <algorithm>

#include "common.h"

using std::string;
using std::vector;

Locator::Locator(const string& filename, const string& input) : filename_(filename), input_(input) {
  int nlines = std::count(input_.begin(), input_.end(), '\n');
  int cur_line = 1;

  linemap_.resize(nlines + 1);
  linemap_[0] = 0;
  for (size_t i = 0; i < input_.length(); ++i) {
    if (input_[i] == '\n') {
      linemap_[cur_line++] = i + 1;
    }
  }
}

void Locator::Locate(Location location, int* line1, int* column1, int* line2, int* column2) const {
  int l1, l2;

#define assign(dst, src) \
  do { \
    if ((dst) != nullptr) { \
      *(dst) = (src); \
    } \
  } while (false)

  using std::upper_bound;

  l1 = upper_bound(linemap_.begin(), linemap_.end(), location.begin) - linemap_.begin() - 1;
  assign(line1, l1);
  assign(column1, location.begin - linemap_[l1]);

  l2 = upper_bound(linemap_.begin(), linemap_.end(), location.end == 0 ? 0 : location.end - 1) - linemap_.begin() - 1;
  assign(line2, l2);
  assign(column2, location.end- linemap_[l2]);

#undef assign
}

void Locator::PrintLocation(int fd, Location location) const {
  FILE* f = fd == 1 ? stdout : stderr;

  fprintf(f, "%s ", filename_.c_str());

  int line1, column1, line2, column2;

  Locate(location, &line1, &column1, &line2, &column2);
  if (line1 == line2) {
    fprintf(f, "%d:%d-%d\n", line1, column1, column2);
  } else {
    fprintf(f, "%d-%d:%d-%d\n", line1, column1, line2, column2);
  }
}

void Locator::Error(int fd, Location location, const string& error) const {
  FILE* f = fd == 1 ? stdout : stderr;

  PrintLocation(fd, location);
  fprintf(f, "error: %s\n", error.c_str());

  int line1, column1, line2, column2;

  Locate(location, &line1, &column1, &line2, &column2);

  // Show one extra line.
  size_t from = line1 == 0 ? 0 : linemap_[line1 - 1];
  size_t to = line1 + 2 < int(linemap_.size()) ? linemap_[line1 + 2] : input_.length();

  for (size_t i = from; i < to; ++i) {
    if (i == location.begin) tty::red(fd);
    fputc(input_[i], f);
    if (i + 1 == location.end) tty::sgr0(fd);
  }
}
