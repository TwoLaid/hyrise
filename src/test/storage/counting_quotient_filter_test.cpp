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
#include "storage/index/counting_quotient_filter/cqf.cpp"

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

TEST_F(CountingQuotientFilterTest, CQFInsert) {
  uint64_t quotient_bits = 16;
  uint64_t remainder_bits = 8;
  uint64_t number_of_slots = std::pow(2, quotient_bits);
  uint64_t hash_bits = quotient_bits + remainder_bits;

  gqf::QF qf;
  uint64_t key = 123456;
  uint64_t count = 1;
  gqf::qf_init(&qf, number_of_slots, hash_bits, 0);
  gqf::qf_insert(&qf, key, 0, count);
  EXPECT_TRUE(gqf::qf_count_key_value(&qf, key, 0) >= 1);
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