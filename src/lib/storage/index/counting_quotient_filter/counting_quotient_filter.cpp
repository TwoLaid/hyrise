#include "counting_quotient_filter.hpp"
#include "utils/murmur_hash.hpp"
#include "utils/xxhash.hpp"

#include <cmath>
#include <iostream>

namespace opossum {

template <typename ElementType>
CountingQuotientFilter<ElementType>::CountingQuotientFilter() {
  _remainders.resize(std::pow(2, sizeof(QuotientType) * 8));
  _occupieds.resize(std::pow(2, sizeof(QuotientType) * 8) / 8);
  _runends.resize(std::pow(2, sizeof(QuotientType) * 8) / 8);
}

template <typename ElementType>
QuotientType CountingQuotientFilter<ElementType>::_find_first_unused_slot(QuotientType quotient) {
  auto rank = _rank(_occupieds, quotient);
  auto select = _select(_runends, rank);

  while (quotient < select) {
    quotient = select + 1;
    rank = _rank(_occupieds, quotient);
    select = _select(_runends, select);
  }

  return quotient;
}

template <typename ElementType>
void CountingQuotientFilter<ElementType>::insert(ElementType element) {
  auto quotient = _hash_quotient(element);
  auto remainder = _hash_remainder(element);
  auto rank = _rank(_occupieds, quotient);
  auto select = _select(_runends, rank);
  if (quotient > select) {
    _remainders[quotient] = remainder;
    _set_bit(_runends, quotient);
  } else {
    ++select;
    auto n = _find_first_unused_slot(select);
    while (n > select) {
      _remainders[n] = _remainders[n - 1];
      _set_bit(_runends, n, _is_bit_set(_runends, n -1));
      --n;
    }
    _remainders[select] = remainder;
    if (_is_bit_set(_occupieds, quotient)) {
      _clear_bit(_runends, select - 1);
    }
    _set_bit(_runends, select);
  }
  _set_bit(_occupieds, quotient);
}

template <typename ElementType>
bool CountingQuotientFilter<ElementType>::lookup(ElementType element) {
  auto quotient = _hash_quotient(element);
  if (!_is_bit_set(_occupieds, quotient)) {
    return false;
  }
  auto t = _rank(_occupieds, quotient);
  auto l = _select(_runends, t);
  auto remainder = _hash_remainder(element);
  do {
    if (_remainders[l] == remainder) {
      return true;
    }
  } while (l >= quotient && !_is_bit_set(_runends, l));
  return false;
}

/**
* Returns the number of 1s in the bit vector up to a certain position.
**/
template <typename ElementType>
int64_t CountingQuotientFilter<ElementType>::_rank(std::vector<uint8_t>& bit_vector, uint64_t position) {
  int64_t rank = 0;
  for (uint64_t i = 0; i <= position; i++) {
    if (_is_bit_set(bit_vector, i)) {
      ++rank;
    }
  }

  return rank;
}

/**
* Returns the position of the n-th 1 in the bit-vector
**/
template <typename ElementType>
int64_t CountingQuotientFilter<ElementType>::_select(std::vector<uint8_t>& bit_vector, uint64_t n) {
  if (n == 0) {
    return -1;
  }

  uint64_t sum = 0;
  for (uint32_t i = 0; i < bit_vector.size() * 8; i++) {
    if (_is_bit_set(bit_vector, i)) {
      ++sum;
      if (sum == n) {
        return static_cast<int64_t>(i);
      }
    }
  }

  return -1;
}

/**
* Returns whether a bit at a certain position in the bit vector is set or not.
**/
template <typename ElementType>
bool CountingQuotientFilter<ElementType>::_is_bit_set(std::vector<uint8_t>& bit_vector, uint64_t position) {
  size_t byte_number = position / 8;
  size_t offset = position % 8;
  return bit_vector[byte_number] & 1 << offset;
}

/**
* Sets the bit at a certain position in a bit vector to 1.
**/
template <typename ElementType>
void CountingQuotientFilter<ElementType>::_set_bit(std::vector<uint8_t>& bit_vector, uint64_t bit) {
    size_t byte_number = bit / 8;
    size_t offset = bit % 8;
    bit_vector[byte_number] = bit_vector[byte_number] | (1 << offset);
}

/**
* Sets the bit at a certain position in a bit vector to the specified value.
**/
template <typename ElementType>
void CountingQuotientFilter<ElementType>::_set_bit(std::vector<uint8_t>& bit_vector, uint64_t position, bool value) {
  if (value) {
    _set_bit(bit_vector, position);
  } else {
    _clear_bit(bit_vector, position);
  }
}

/**
* Clears the bit at a certain position in a bit vector.
**/
template <typename ElementType>
void CountingQuotientFilter<ElementType>::_clear_bit(std::vector<uint8_t>& bit_vector, uint64_t position) {
    size_t byte_number = position / 8;
    size_t offset = position % 8;
    bit_vector[byte_number] = bit_vector[byte_number] & ~(1 << offset);
}

/**
* Computes the quotient part of a value hash.
**/
template <typename ElementType>
QuotientType CountingQuotientFilter<ElementType>::_hash_quotient(ElementType value) {
  uint32_t seed = 384812094;
  auto hash = xxh::xxhash<32, ElementType>(&value, 1, seed);
  return static_cast<QuotientType>(hash);
}

/**
* Computes the remainder part of a value hash.
**/
template <typename ElementType>
RemainderType CountingQuotientFilter<ElementType>::_hash_remainder(ElementType value) {
  uint32_t seed = 93498548;
  auto hash = xxh::xxhash<32, ElementType>(&value, 1, seed);
  return static_cast<RemainderType>(hash);
}

/**
* Returns the number of occupied slots in the remainder array.
**/
template <typename ElementType>
uint64_t CountingQuotientFilter<ElementType>::number_of_occupied_slots() {
  return _rank(_occupieds, static_cast<QuotientType>(-1));
}

} // namespace opossum