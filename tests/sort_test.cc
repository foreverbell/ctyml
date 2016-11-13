#include <algorithm>

#include <gtest/gtest.h>

TEST(SortTest, Ascending) {
  std::vector<int> vec = {4, 2, 5, 1};

  std::sort(vec.begin(), vec.end());
  for (int i = 0; i < vec.size() - 1; ++i) {
    EXPECT_LT(vec[i], vec[i + 1]);
  }
}
