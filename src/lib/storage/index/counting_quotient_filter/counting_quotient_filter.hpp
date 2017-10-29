#pragma once

#include "cqf.hpp"

#include <cstdint>
#include <vector>

namespace opossum {

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
  void insert(ElementType value, uint64_t count);
  void insert(ElementType value);
  uint64_t count(ElementType value);

 private:
  gqf::quotient_filter _quotient_filter;
  uint64_t _hash(ElementType value);

};

} // namespace opossum