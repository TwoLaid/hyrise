
namespace opossum {

/**
Following the idea and implementation of Pandey, Johnson and Patro:
Paper: A General-Purpose Counting Filter: Making Every Bit Count
Repository: https://github.com/splatlab/cqf
**/
template <typename ElementType>
class CountingQuotientFilter
{
 using QuotientType = uint32_t;
 using RemainderType = uint8_t;

 public:
  CountingQuotientFilter();
  void insert(ElementType element);
  bool lookup();

 private:
  std::pair<QuotientType, RemainderType> hash(ElementType);
  bool is_slot_occupied(QuotientType slot_id);
  bool is_slot_runend(QuotientType slot_id);
  void set_slot_occupied(QuotientType slot_id);
  void set_slot_runend(QuotientType slot_id);
  void clear_slot_occupied(QuotientType slot_id);
  void clear_slot_runend(QuotientType slot_id);

  std::vector<uint8_t> _occupieds;
  std::vector<uint8_t> _runends;
  std::vector<RemainderType> _remainders;
};

} // namespace opossum