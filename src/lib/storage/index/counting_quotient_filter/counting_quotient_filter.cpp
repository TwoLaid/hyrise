#include "counting_quotient_filter.hpp"

CountingQuotientFilter::CountingQuotientFilter() {
  _remainders.resize(std::pow(2, sizeof(QuotientType)));
}

void CountingQuotientFilter::insert(ElementType element) {
}

std::pair<QuotientType, RemainderType> CountingQuotientFilter::hash(ElementType) {
  // TODO
  return std::pair<QuotientType, RemainderType>(0u, 0u);
}

bool CountingQuotientFilter::is_slot_occupied(QuotientType slot_id) {
  size_t byte_number = slot_id / 8;
  size_t offset = slot_id % 8;
  return _occupieds[byte_number] & 1 << offset;
}

bool CountingQuotientFilter::is_slot_runend(QuotientType slot_id) {
    size_t byte_number = slot_id / 8;
    size_t offset = slot_id % 8;
    return _runends[byte_number] & 1 << offset;
}