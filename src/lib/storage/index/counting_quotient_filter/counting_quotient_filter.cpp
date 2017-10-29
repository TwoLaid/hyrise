#include "counting_quotient_filter.hpp"
#include "cqf.hpp"
#include "utils/xxhash.hpp"

#include <cmath>
#include <iostream>

namespace opossum {

template <typename ElementType>
CountingQuotientFilter<ElementType>::CountingQuotientFilter() {
  uint64_t quotient_bits = 16;
  uint64_t remainder_bits = 8;
  uint64_t number_of_slots = std::pow(2, quotient_bits);
  uint64_t hash_bits = quotient_bits + remainder_bits;

  gqf::qf_init(&_quotient_filter, number_of_slots, hash_bits, 0);
}

template <typename ElementType>
void CountingQuotientFilter<ElementType>::insert(ElementType element, uint64_t count) {
  uint16_t hash = static_cast<uint16_t>(_hash(element));
  gqf::qf_insert(&_quotient_filter, hash, 0, count);
}

template <typename ElementType>
void CountingQuotientFilter<ElementType>::insert(ElementType element) {
  insert(element, 1);
}

template <typename ElementType>
uint64_t CountingQuotientFilter<ElementType>::count(ElementType element) {
  uint16_t hash = static_cast<uint16_t>(_hash(element));
  return gqf::qf_count_key_value(&_quotient_filter, hash, 0);
}

/**
* Computes the hash for a value.
**/
template <typename ElementType>
uint64_t CountingQuotientFilter<ElementType>::_hash(ElementType value) {
  uint32_t seed = 384812094;
  auto hash = xxh::xxhash<64, ElementType>(&value, 1, seed);
  return static_cast<uint64_t>(hash);
}

} // namespace opossum