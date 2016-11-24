#pragma once

#include <string>
#include <sstream>

inline std::vector<std::string> SplitByLine(const std::string& input) {
  std::stringstream ss(input);
  std::vector<std::string> cuts;

  while (true) {
    std::string tmp;
    if (std::getline(ss, tmp)) {
      if (tmp.empty()) continue;
      cuts.push_back(tmp);
    } else break;
  }
  return cuts;
}
