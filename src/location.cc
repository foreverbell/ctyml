#include "location.h"

#include <algorithm>

using std::string;
using std::vector;

Locator::Locator(const string& input) : input_(input) {
  int nlines = std::count(input_.begin(), input_.end(), '\n');
  int cur_line = 1;

  linemap_.resize(nlines + 1);
  linemap_[0] = 0;
  for (int i = 0; i < input_.length(); ++i) {
    if (input_[i] == '\n') {
      linemap_[cur_line++] = i + 1;
    }
  }
}

void Locator::Locate(Location location, int* line1, int* column1, int* line2, int* column2) {
  int l1, l2;
  // Find the largest index that <= location.begin (end).
  l1 = std::upper_bound(linemap_.begin(), linemap_.end(), location.begin) - linemap_.begin() - 1;
  l2 = std::upper_bound(linemap_.begin(), linemap_.end(), std::max(location.end - 1, 0l)) - linemap_.begin() - 1;

#define assign(dst, src) \
  do { \
    if ((dst) != nullptr) { \
      *(dst) = (src); \
    } \
  } while (false)

  assign(line1, l1);
  assign(line2, l2);
  assign(column1, location.begin - linemap_[l1]);
  assign(column2, location.end- linemap_[l2]);

#undef assign
}
