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
  std::cout << "before insert" << std::endl;
  filter.insert(7);
  std::cout << "after insert" << std::endl;
  EXPECT_EQ(6u, 6u);
}

TEST_F(CountingQuotientFilterTest, Membership) {
}

} // namespace opossum