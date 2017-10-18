#include "counting_quotient_filter.hpp"

CountingQuotientFilter::CountingQuotientFilter() {
  _remainders.resize(std::pow(2, sizeof(QuotientType)));
}

size_t CountingQuotientFilter::find_first_unused_slot(x) {
  return 0;
}

void CountingQuotientFilter::insert(ElementType element) {
  auto quotient = hash_quotient(element);
  auto r = rank()
}

bool CountingQuotientFilter::lookup(ElementType element) {
  auto quotient = hash_quotient(element);
  if (!is_slot_occupied(quotient)) {
    return false;
  }
  auto t = rank(_occupieds, quotient);
  auto l = select(_runends, t);
  auto remainder = hash_remainder(element);
  do {
    if (_remainders[l] == remainder) {
      return true;
    }
  } while (!(l < b || is_slot_runend(l)));
  return false;
}

QuotientType CountingQuotientFilter::rank(std::vector<uint8_t>& bit_vector, ??) {
}

QuotientType CountingQuotientFilter::select(std::vector<uint8_t>& bit_vector, ??) {
}

QuotientType CountingQuotientFilter::hash_quotient(ElementType) {
  return 0u;
}

RemainderType CountingQuotientFilter::hash_remainder(ElementType) {
  return 0u;
}

bool CountingQuotientFilter::is_slot_occupied(QuotientType slot_id) {
  size_t byte_number = slot_id / 8;
  size_t offset = slot_id % 8;
  return _occupieds[byte_number] & 1 << offset;
}

bool CountingQuotientFilter::is_slot_runend(QuotientType slot_id) {
  size_t byte_number = slot_id / 8;
  size_t offset = slot_id % 8;
  return _runends[byte_number] & (1 << offset);
}

void CountingQuotientFilter::set_slot_occupied(QuotientType slot_id) {
  size_t byte_number = slot_id / 8;
  size_t offset = slot_id % 8;
  _occupieds[byte_number] = _occupieds[byte_number] | (1 << offset);
}

void CountingQuotientFilter::set_slot_runend(QuotientType slot_id) {
  size_t byte_number = slot_id / 8;
  size_t offset = slot_id % 8;
  _runends[byte_number] = _runends[byte_number] | (1 << offset);
}

void CountingQuotientFilter::clear_slot_occupied(QuotientType slot_id) {
  size_t byte_number = slot_id / 8;
  size_t offset = slot_id % 8;
  _occupieds[byte_number] = _occupieds[byte_number] & ~(1 << offset);
}

void CountingQuotientFilter::clear_slot_runend(QuotientType slot_id) {
  size_t byte_number = slot_id / 8;
  size_t offset = slot_id % 8;
  _occupieds[byte_number] = _occupieds[byte_number] & ~(1 << offset);
}