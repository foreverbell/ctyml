#pragma once

#include <memory>
#include <vector>

#include "token.h"



bool ParseToken(const std::vector<std::unique_ptr<Token>>& tokens);
