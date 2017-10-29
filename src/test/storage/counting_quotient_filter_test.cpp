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

TEST_F(CountingQuotientFilterTest, Insert) {
  auto filter = CountingQuotientFilter<int>();
  filter.insert(7);
  EXPECT_EQ(filter.number_of_occupied_slots(), 1u);
}

TEST_F(CountingQuotientFilterTest, Membership) {
  auto filter = CountingQuotientFilter<int>();
  filter.insert(7);
  EXPECT_TRUE(filter.lookup(7));
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
  EXPECT_TRUE(filter.lookup(10));
  EXPECT_TRUE(filter.lookup(20));
  EXPECT_TRUE(filter.lookup(30));
  EXPECT_TRUE(filter.lookup(40));
  EXPECT_TRUE(filter.lookup(50));
  EXPECT_TRUE(filter.lookup(60));
  EXPECT_TRUE(filter.lookup(70));
  EXPECT_TRUE(filter.lookup(80));
  EXPECT_TRUE(filter.lookup(90));
  EXPECT_TRUE(filter.lookup(100));
}

} // namespace opossum