#pragma once

#include <memory>
#include <string>
#include <vector>

#include "syntax.h"
#include "token.h"

bool ScanTokens(const std::vector<std::string>& input, std::vector<std::unique_ptr<Token>>* tokens);
