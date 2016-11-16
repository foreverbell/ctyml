#include "lexer.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

using std::string;
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
  TokenType type = id[0] >= 'a' && id[0] <= 'z' ? TokenType::LCaseId : TokenType::UCaseId;
  return std::make_tuple(type, -1, id);
}

class LexerTest : public ::testing::Test {
 protected:
  void test(const string& input, const vector<token_tuple>& expected) {
    lexer.reset(Lexer::Create(input));
    ASSERT_TRUE(lexer != nullptr);

    ASSERT_EQ(expected.size(), lexer->size());
    for (size_t i = 0; i < expected.size(); ++i) {
      const Token* token = lexer->get(i);
      EXPECT_EQ(std::get<0>(expected[i]), token->type());
      if (token->is_int()) {
        EXPECT_EQ(std::get<1>(expected[i]), token->number());
      }
      if (token->is_id()) {
        EXPECT_EQ(std::get<2>(expected[i]), token->identifier());
      }
    }
  }

 private:
  unique_ptr<Lexer> lexer;
};

TEST_F(LexerTest, KeywordsTest) {
  test(R"(if true then false else true;)",
       {create(TokenType::If), create(TokenType::True), create(TokenType::Then), create(TokenType::False),
        create(TokenType::Else), create(TokenType::True), create(TokenType::Semi)});
}

TEST_F(LexerTest, SymbolsTest) {
  test(R"(.,:;=_->(){}[])",
       {create(TokenType::Dot), create(TokenType::Comma), create(TokenType::Colon), create(TokenType::Semi),
        create(TokenType::Eq), create(TokenType::UScore), create(TokenType::Arrow), create(TokenType::LParen),
        create(TokenType::RParen), create(TokenType::LCurly), create(TokenType::RCurly), create(TokenType::LBracket),
        create(TokenType::RBracket)});
}

TEST_F(LexerTest, VariablesTest) {
  test(R"(if false then x else y;)",
       {create(TokenType::If), create(TokenType::False), create(TokenType::Then), create_id("x"),
        create(TokenType::Else), create_id("y"), create(TokenType::Semi)});
}

TEST_F(LexerTest, NumbersTest) {
  test(R"(123;)",
       {create_int(123), create(TokenType::Semi)});
}

TEST_F(LexerTest, MixtureTest) {
  test(R"(
    (lambda x. x) 42;
    type T = Nat->Nat;
       )",
       {create(TokenType::LParen), create(TokenType::Lambda), create_id("x"), create(TokenType::Dot), create_id("x"),
        create(TokenType::RParen), create_int(42), create(TokenType::Semi), create(TokenType::TypeAlias),
        create_id("T"), create(TokenType::Eq), create(TokenType::Nat), create(TokenType::Arrow), create(TokenType::Nat),
        create(TokenType::Semi)});
}

// TODO(foreverbell): Test lexical error.
