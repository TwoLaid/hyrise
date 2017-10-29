#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#include <iostream>

#include "../base_test.hpp"
#include "gtest/gtest.h"
#include "types.hpp"

#include "storage/dictionary_column.hpp"
#include "storage/index/counting_quotient_filter/counting_quotient_filter.cpp"

namespace opossum {

class CountingQuotientFilterTest : public BaseTest {
 protected:
  void SetUp () override {
  }
};

TEST_F(CountingQuotientFilterTest, Membership) {
  auto filter = CountingQuotientFilter<int>();
  filter.insert(12345);
  EXPECT_TRUE(filter.count(12345) >= 1);
}

TEST_F(CountingQuotientFilterTest, MultipleMembership) {
  auto filter = CountingQuotientFilter<int>();
  filter.insert(10);
  filter.insert(20);
  filter.insert(30);
  filter.insert(40);
  filter.insert(50);
  filter.insert(60);
  filter.insert(70);
  filter.insert(80);
  filter.insert(90);
  filter.insert(100);
  EXPECT_TRUE(filter.count(10) >= 1);
  EXPECT_TRUE(filter.count(20) >= 1);
  EXPECT_TRUE(filter.count(30) >= 1);
  EXPECT_TRUE(filter.count(40) >= 1);
  EXPECT_TRUE(filter.count(50) >= 1);
  EXPECT_TRUE(filter.count(60) >= 1);
  EXPECT_TRUE(filter.count(70) >= 1);
  EXPECT_TRUE(filter.count(80) >= 1);
  EXPECT_TRUE(filter.count(90) >= 1);
  EXPECT_TRUE(filter.count(100) >= 1);
}

} // namespace opossum