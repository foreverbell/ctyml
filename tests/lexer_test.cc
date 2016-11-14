#include "lexer.h"

#include <gtest/gtest.h>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

using std::string;
using std::stringstream;
using std::tuple;
using std::unique_ptr;
using std::vector;

typedef tuple<TokenType, int, string> token_tuple;

token_tuple create(TokenType type) {
  return std::make_tuple(type, -1, "");
}

token_tuple create_int(int num) {
  return std::make_tuple(TokenType::Int, num, "");
}

token_tuple create_id(const string& id) {
  TokenType type = id[0] >= 'A' && id[0] <= 'Z' ? TokenType::LCaseId : TokenType::UCaseId;
  return std::make_tuple(type, -1, id);
}

class LexerTest : public ::testing::Test {
 protected:
  void SetUp() override { }

  void test(const string& input, const vector<token_tuple>& expected) {
    stringstream ss(input);
    vector<string> inputs;
    string buffer;
    vector<unique_ptr<Token>> tokens;

    while (std::getline(ss, buffer)) {
      inputs.push_back(std::move(buffer));
    }
    ASSERT_TRUE(ScanTokens(inputs, &tokens));
    ASSERT_EQ(expected.size(), tokens.size());
    for (int i = 0; i < expected.size(); ++i) {
      EXPECT_EQ(std::get<0>(expected[i]), tokens[i]->type());
      if (tokens[i]->is_int()) {
        EXPECT_EQ(std::get<1>(expected[i]), tokens[i]->number());
      }
      if (tokens[i]->is_id()) {
        EXPECT_EQ(std::get<2>(expected[i]), tokens[i]->identifier());
      }
    }
  }
};

TEST_F(LexerTest, Keyword) {
  test(R"(if true then false else true)",
       {create(TokenType::If), create(TokenType::True), create(TokenType::Then), create(TokenType::False),
        create(TokenType::Else), create(TokenType::True)});
}
