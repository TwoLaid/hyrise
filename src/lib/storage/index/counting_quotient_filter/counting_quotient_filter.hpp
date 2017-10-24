#pragma once

#include <cstdint>
#include <vector>

namespace opossum {

using QuotientType = uint16_t;
using RemainderType = uint16_t;

/**
Following the idea and implementation of Pandey, Johnson and Patro:
Paper: A General-Purpose Counting Filter: Making Every Bit Count
Repository: https://github.com/splatlab/cqf
**/
template <typename ElementType>
class CountingQuotientFilter
{
 public:
  CountingQuotientFilter();
  void insert(ElementType value);
  bool lookup(ElementType value);

 private:
  QuotientType hash_quotient(ElementType value);
  RemainderType hash_remainder(ElementType value);
  QuotientType rank(std::vector<uint8_t>& bit_vector, QuotientType position);
  QuotientType select(std::vector<uint8_t>& bit_vector, QuotientType n);
  bool is_bit_set(std::vector<uint8_t>& bit_vector, size_t bit);
  void set_bit(std::vector<uint8_t>& bit_vector, size_t bit);
  void set_bit(std::vector<uint8_t>& bit_vector, size_t bit, bool value);
  void clear_bit(std::vector<uint8_t>& bit_vector, size_t bit);
  QuotientType find_first_unused_slot(QuotientType quotient);

  std::vector<uint8_t> _occupieds;
  std::vector<uint8_t> _runends;
  std::vector<RemainderType> _remainders;
};

} // namespace opossum