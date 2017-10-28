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

} // namespace opossum