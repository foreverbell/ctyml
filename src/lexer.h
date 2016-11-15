#pragma once

#include <memory>
#include <string>
#include <vector>

#include "common.h"
#include "token.h"

// TODO(foreverbell): set returning type to void and throw an exception?
bool ScanTokens(const std::vector<std::string>& input, std::vector<std::unique_ptr<Token>>* tokens);
