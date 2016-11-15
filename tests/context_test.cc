#include "context.h"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

class ContextTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ctx_.AddName("x");
  }

  Context ctx_;
};

TEST_F(ContextTest, GenericTest) {
  EXPECT_EQ(ctx_.size(), 1);
  EXPECT_EQ(ctx_.get(0).first, "x");
  EXPECT_EQ(ctx_.ToIndex("x"), 0);

  ctx_.AddName("y");
  EXPECT_EQ(ctx_.size(), 2);
  EXPECT_EQ(ctx_.get(0).first, "y");
  EXPECT_EQ(ctx_.get(1).first, "x");
  EXPECT_EQ(ctx_.ToIndex("x"), 1);
  EXPECT_EQ(ctx_.ToIndex("y"), 0);

  // Duplicate name is allowed.
  ctx_.AddName("x");
  EXPECT_EQ(ctx_.size(), 3);
  EXPECT_EQ(ctx_.get(0).first, "x");
  EXPECT_EQ(ctx_.get(1).first, "y");
  EXPECT_EQ(ctx_.get(2).first, "x");
  EXPECT_EQ(ctx_.ToIndex("y"), 1);
  EXPECT_EQ(ctx_.ToIndex("x"), 0);

  ctx_.AddName("z");
  ctx_.DropBindings(2);
  EXPECT_EQ(ctx_.get(0).first, "y");
  EXPECT_EQ(ctx_.get(1).first, "x");
  EXPECT_EQ(ctx_.ToIndex("x"), 1);
  EXPECT_EQ(ctx_.ToIndex("y"), 0);
}

TEST_F(ContextTest, FreshNameTest) {
  ctx_.AddName("x_1");
  ctx_.AddName("x_11");
  ctx_.AddName("x_2");
  ctx_.AddName("x_5");
  ctx_.AddName("x_0");
  ctx_.AddName("y");
  ctx_.AddName("foreverbell");

  EXPECT_EQ(ctx_.size(), 8);

  EXPECT_EQ(ctx_.PickFreshName("x"), "x_3");
  EXPECT_EQ(ctx_.size(), 9);

  EXPECT_EQ(ctx_.PickFreshName("z"), "z");
  EXPECT_EQ(ctx_.size(), 10);
}
