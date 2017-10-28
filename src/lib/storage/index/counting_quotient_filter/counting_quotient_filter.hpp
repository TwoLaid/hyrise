#pragma once

#include <cstdint>
#include <vector>

namespace opossum {

using QuotientType = uint8_t;
using RemainderType = uint8_t;

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
  QuotientType _hash_quotient(ElementType value);
  RemainderType _hash_remainder(ElementType value);
  int64_t _rank(std::vector<uint8_t>& bit_vector, QuotientType position);
  int64_t _select(std::vector<uint8_t>& bit_vector, int64_t n);
  bool _is_bit_set(std::vector<uint8_t>& bit_vector, size_t bit);
  void _set_bit(std::vector<uint8_t>& bit_vector, size_t bit);
  void _set_bit(std::vector<uint8_t>& bit_vector, size_t bit, bool value);
  void _clear_bit(std::vector<uint8_t>& bit_vector, size_t bit);
  QuotientType _find_first_unused_slot(QuotientType quotient);

  std::vector<uint8_t> _occupieds;
  std::vector<uint8_t> _runends;
  std::vector<RemainderType> _remainders;
};

} // namespace opossum