#include "counting_quotient_filter.hpp"
#include "utils/murmur_hash.hpp"
#include "utils/xxhash.hpp"

#include <cmath>

namespace opossum {

template <typename ElementType>
CountingQuotientFilter<ElementType>::CountingQuotientFilter() {
  _remainders.resize(std::pow(2, sizeof(QuotientType)));
  _occupieds.resize(std::pow(2, sizeof(QuotientType)) / 8);
  _runends.resize(std::pow(2, sizeof(QuotientType)) / 8);
}

template <typename ElementType>
QuotientType CountingQuotientFilter<ElementType>::find_first_unused_slot(QuotientType quotient) {
  auto rank = rank(_occupieds, quotient);
  auto select = select(_runends, rank);

  while (quotient < select) {
    quotient = select + 1;
    rank = rank(_occupieds, quotient);
    select = select(_runends, select);
  }

  return quotient;
}

template <typename ElementType>
void CountingQuotientFilter<ElementType>::insert(ElementType element) {
  auto quotient = hash_quotient(element);
  auto remainder = hash_remainder(element);
  auto rank = rank(_occupieds, quotient);
  auto select = select(_runends, rank);
  if (quotient > select) {
    _remainders[quotient] = remainder;
    set_bit(_runends, quotient);
  } else {
    ++select;
    auto n = find_first_unused_slot(select);
    while (n > select) {
      _remainders[n] = _remainders[n - 1];
      set_bit(_runends, n, is_bit_set(_runends, n -1));
      --n;
    }
    _remainders[select] = remainder;
    if (is_bit_set(_occupieds, quotient)) {
      clear_bit(_runends, select - 1);
    }
    set_bit(_runends, select);
  }
  set_bit(_occupieds, quotient);
}

template <typename ElementType>
bool CountingQuotientFilter<ElementType>::lookup(ElementType element) {
  auto quotient = hash_quotient(element);
  if (!is_bit_set(_occupieds, quotient)) {
    return false;
  }
  auto t = rank(_occupieds, quotient);
  auto l = select(_runends, t);
  auto remainder = hash_remainder(element);
  do {
    if (_remainders[l] == remainder) {
      return true;
    }
  } while (l >= b && !is_bit_set(_runends, l));
  return false;
}

/**
* Returns the number of 1s in the bit vector up to a certain position.
**/
template <typename ElementType>
QuotientType CountingQuotientFilter<ElementType>::rank(std::vector<uint8_t>& bit_vector, QuotientType position) {
  QuotientType rank = 0;
  for (QuotientType i = 0; i <= position; i++) {
    if (is_bit_set(bit_vector, i)) {
      ++rank;
    }
  }

  return rank;
}

/**
* Returns the position of the n-th 1 in the bit-vector
**/
template <typename ElementType>
QuotientType CountingQuotientFilter<ElementType>::select(std::vector<uint8_t>& bit_vector, QuotientType n) {
  QuotientType sum = 0;
  for (QuotientType i = 0; i <= bit_vector.size() * 8; i++) {
    if (is_bit_set(bit_vector, i)) {
      ++sum;
      if (sum == n) {
        return i;
      }
    }
  }

  throw std::logic_error("select did not find the n-th 1 in the bit vector");
}

/**
* Returns whether a bit at a certain position in the bit vector is set or not.
**/
template <typename ElementType>
bool CountingQuotientFilter<ElementType>::is_bit_set(std::vector<uint8_t>& bit_vector, size_t position) {
  size_t byte_number = position / 8;
  size_t offset = position % 8;
  return bit_vector[byte_number] & 1 << offset;
}

/**
* Sets the bit at a certain position in a bit vector to 1.
**/
template <typename ElementType>
void CountingQuotientFilter<ElementType>::set_bit(std::vector<uint8_t>& bit_vector, size_t bit) {
    size_t byte_number = bit / 8;
    size_t offset = bit % 8;
    bit_vector[byte_number] = bit_vector[byte_number] | (1 << offset);
}

/**
* Sets the bit at a certain position in a bit vector to the specified value.
**/
template <typename ElementType>
void CountingQuotientFilter<ElementType>::set_bit(std::vector<uint8_t>& bit_vector, size_t bit, bool value) {
  if (value) {
    set_bit(bit_vector, bit);
  } else {
    clear_bit(bit_vector, bit);
  }
}

/**
* Clears the bit at a certain position in a bit vector.
**/
template <typename ElementType>
void CountingQuotientFilter<ElementType>::clear_bit(std::vector<uint8_t>& bit_vector, size_t bit) {
    size_t byte_number = bit / 8;
    size_t offset = bit % 8;
    bit_vector[byte_number] = bit_vector[byte_number] & ~(1 << offset);
}

/**
* Computes the quotient part of a value hash.
**/
template <typename ElementType>
QuotientType CountingQuotientFilter<ElementType>::hash_quotient(ElementType value) {
  // arbitrary seed for the first hash iteration
  unsigned int seed = 13;
  auto hash = murmur2<ElementType>(value, seed);
  return static_cast<QuotientType>(hash);
}

/**
* Computes the remainder part of a value hash.
**/
template <typename ElementType>
RemainderType CountingQuotientFilter<ElementType>::hash_remainder(ElementType value) {
  auto hash = xxh::xxhash<32, ElementType>(&value, 1);
  return static_cast<RemainderType>(hash);
}

} // namespace opossum