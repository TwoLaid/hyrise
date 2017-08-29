#include "join_sort_merge.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "resolve_type.hpp"
#include "join_sort_merge_utils/radix_partition_sort.hpp"

namespace opossum {

/**
* TODO(arne.mayer): Outer not-equal join (outer !=)
* TODO(anyone): Choose an appropriate number of partitions.
**/

/**
* The sort merge join performs a join on two input tables on specific join columns. For usage notes, see the
* join_sort_merge.hpp. This is how the join works:
* -> The input tables are materialized and partitioned to a specified amount of partitions.
*    /utils/radix_partition_sort.hpp for more info on the partitioning phase.
* -> The join is performed per partition. For the joining phase, runs of entries with the same value are identified
*    and handled at once. If a join-match is identified, the corresponding row_ids are noted for the output.
* -> Using the join result, the output table is built using pos lists referencing the original tables.
**/
JoinSortMerge::JoinSortMerge(const std::shared_ptr<const AbstractOperator> left,
                             const std::shared_ptr<const AbstractOperator> right,
                             optional<std::pair<std::string, std::string>> column_names, const ScanType op,
                             const JoinMode mode, const std::string& prefix_left, const std::string& prefix_right)
    : AbstractJoinOperator(left, right, column_names, op, mode, prefix_left, prefix_right) {
  // Validate the parameters
  DebugAssert(mode != JoinMode::Cross && column_names, "This operator does not support cross joins.");
  DebugAssert(left != nullptr, "The left input operator is null.");
  DebugAssert(right != nullptr, "The right input operator is null.");
  DebugAssert(op == ScanType::OpEquals || op == ScanType::OpLessThan || op == ScanType::OpGreaterThan ||
              op == ScanType::OpLessThanEquals || op == ScanType::OpGreaterThanEquals, "Unsupported scan type");
  DebugAssert(op == ScanType::OpEquals || mode == JoinMode::Inner, "Outer joins are only implemented for equi joins.");

  auto left_column_name = column_names->first;
  auto right_column_name = column_names->second;

  // Check column types
  const auto left_column_id = input_table_left()->column_id_by_name(left_column_name);
  const auto& left_column_type = input_table_left()->column_type(left_column_id);

  DebugAssert(left_column_type == input_table_right()->column_type(
                                                          input_table_right()->column_id_by_name(right_column_name)),
              "Left and right column types do not match. The sort merge join requires matching column types");

  // Create implementation to compute the join result
  _impl = make_unique_by_column_type<AbstractJoinOperatorImpl, JoinSortMergeImpl>(left_column_type,
    *this, left_column_name, right_column_name, op, mode);
}

std::shared_ptr<AbstractOperator> JoinSortMerge::recreate(const std::vector<AllParameterVariant> &args) const {
  Fail("Operator " + this->name() + " does not implement recreation.");
  return {};
}

std::shared_ptr<const Table> JoinSortMerge::on_execute() { return _impl->on_execute(); }

const std::string JoinSortMerge::name() const { return "JoinSortMerge"; }

uint8_t JoinSortMerge::num_in_tables() const { return 2u; }

uint8_t JoinSortMerge::num_out_tables() const { return 1u; }

/**
** Start of implementation.
**/
template <typename T>
class JoinSortMerge::JoinSortMergeImpl : public AbstractJoinOperatorImpl {
 public:
  JoinSortMergeImpl<T>(JoinSortMerge& sort_merge_join, std::string left_column_name,
                       std::string right_column_name, const ScanType op, JoinMode mode)
    : _sort_merge_join{sort_merge_join}, _left_column_name{left_column_name}, _right_column_name{right_column_name},
      _op{op}, _mode{mode} {

    _partition_count = _determine_number_of_partitions();
    _output_pos_lists_left.resize(_partition_count);
    _output_pos_lists_right.resize(_partition_count);
  }

 protected:
  using MatTablePtr = std::shared_ptr<MaterializedTable<T>>;

  JoinSortMerge& _sort_merge_join;

  // Contains the materialized sorted input tables
  MatTablePtr _sorted_left_table;
  MatTablePtr _sorted_right_table;

  const std::string _left_column_name;
  const std::string _right_column_name;

  const ScanType _op;
  const JoinMode _mode;

  // the partition count must be a power of two, i.e. 1, 2, 4, 8, 16, ...
  size_t _partition_count;

  // Contains the output row ids for each partition
  std::vector<std::shared_ptr<PosList>> _output_pos_lists_left;
  std::vector<std::shared_ptr<PosList>> _output_pos_lists_right;

  /**
   * The TablePosition is a utility struct that is used to define a specific position in a sorted input table.
  **/
  struct TableRange;
  struct TablePosition {
    TablePosition() {}
    TablePosition(size_t partition, size_t index) : partition{partition}, index{index} {}

    size_t partition;
    size_t index;

    TableRange to(TablePosition position) {
      return TableRange(*this, position);
    }
  };

  TablePosition _end_of_left_table;
  TablePosition _end_of_right_table;

  /**
    * The TableRange is a utility struct that is used to define ranges of rows in a sorted input table spanning from
    * a start position to an end position.
  **/
  struct TableRange {
    TableRange(TablePosition start_position, TablePosition end_position) : start(start_position), end(end_position) {}
    TableRange(size_t partition, size_t start_index, size_t end_index)
      : start{TablePosition(partition, start_index)}, end{TablePosition(partition, end_index)} {}

    TablePosition start;
    TablePosition end;

    // Executes the given action for every row id of the table in this range.
    template <typename F>
    void for_every_row_id(MatTablePtr table, F action) {
      for (size_t partition = start.partition; partition <= end.partition; ++partition) {
        size_t start_index = (partition == start.partition) ? start.index : 0;
        size_t end_index = (partition == end.partition) ? end.index : (*table)[partition]->size();
        for (size_t index = start_index; index < end_index; ++index) {
          action((*(*table)[partition])[index].row_id);
        }
      }
    }
  };

  /**
  * Determines the number of partitions to be used for the join.
  * The number of partitions must be a power of two, i.e. 1, 2, 4, 8, 16...
  * TODO(anyone): How should we determine the number of partitions?
  **/
  size_t _determine_number_of_partitions() {
    // Get the next lower power of two of the bigger chunk number
    // Note: this is only provisional. There should be a reasonable calculation here based on hardware stats.
    size_t chunk_count_left = _sort_merge_join.input_table_left()->chunk_count();
    size_t chunk_count_right = _sort_merge_join.input_table_right()->chunk_count();
    return static_cast<size_t>(std::pow(2, std::floor(std::log2(std::max(chunk_count_left, chunk_count_right)))));
  }

  /**
  * Gets the table position corresponding to the end of the table, i.e. the last entry of the last partition.
  **/
  static TablePosition _end_of_table(MatTablePtr table) {
    DebugAssert(table->size() > 0, "table has no chunks");
    auto last_partition = table->size() - 1;
    return TablePosition(last_partition, (*table)[last_partition]->size());
  }

  /**
  * Represents the result of a value comparison.
  **/
  enum class CompareResult {
    Less,
    Greater,
    Equal
  };

  /**
  * Performs the join for two runs of a specified partition.
  * A run is a series of rows in a partition with the same value.
  **/
  void _join_runs(TableRange left_run, TableRange right_run, CompareResult compare_result) {
    size_t partition_number = left_run.start.partition;
    if (_op == ScanType::OpEquals) {
      if (compare_result == CompareResult::Equal) {
        _emit_combinations(partition_number, left_run, right_run);
      } else if (compare_result == CompareResult::Less) {
        if (_mode == JoinMode::Left || _mode == JoinMode::Outer) {
          _emit_right_null_combinations(partition_number, left_run);
        }
      } else if (compare_result == CompareResult::Greater) {
        if (_mode == JoinMode::Right || _mode == JoinMode::Outer) {
          _emit_left_null_combinations(partition_number, right_run);
        }
      }
    } else if (_op == ScanType::OpNotEquals) {
      if (compare_result == CompareResult::Greater) {
        _emit_combinations(partition_number, left_run.start.to(_end_of_left_table), right_run);
      } else if (compare_result == CompareResult::Equal) {
        _emit_combinations(partition_number, left_run.end.to(_end_of_left_table), right_run);
        _emit_combinations(partition_number, left_run, right_run.end.to(_end_of_right_table));
      } else if (compare_result == CompareResult::Less) {
        _emit_combinations(partition_number, left_run, right_run.start.to(_end_of_right_table));
      }
    } else if (_op == ScanType::OpGreaterThan) {
      if (compare_result == CompareResult::Greater) {
        _emit_combinations(partition_number, left_run.start.to(_end_of_left_table), right_run);
      } else if (compare_result == CompareResult::Equal) {
        _emit_combinations(partition_number, left_run.end.to(_end_of_left_table), right_run);
      }
    } else if (_op == ScanType::OpGreaterThanEquals) {
      if (compare_result == CompareResult::Greater || compare_result == CompareResult::Equal) {
        _emit_combinations(partition_number, left_run.start.to(_end_of_left_table), right_run);
      }
    } else if (_op == ScanType::OpLessThan) {
      if (compare_result == CompareResult::Less) {
        _emit_combinations(partition_number, left_run, right_run.start.to(_end_of_right_table));
      } else if (compare_result == CompareResult::Equal) {
        _emit_combinations(partition_number, left_run, right_run.end.to(_end_of_right_table));
      }
    } else if (_op == ScanType::OpLessThanEquals) {
      if (compare_result == CompareResult::Less || compare_result == CompareResult::Equal) {
        _emit_combinations(partition_number, left_run, right_run.start.to(_end_of_right_table));
      }
    } else {
      throw std::logic_error("Unknown ScanType");
    }
  }

  /**
  * Emits a combination of a lhs row id and a rhs row id to the join output.
  **/
  void _emit_combination(size_t output_partition, const RowID& left, const RowID& right) {
    _output_pos_lists_left[output_partition]->push_back(left);
    _output_pos_lists_right[output_partition]->push_back(right);
  }

  /**
  * Emits all the combinations of row ids from the left table range and the right table range to the join output.
  * I.e. the cross product of the ranges is emitted.
  **/
  void _emit_combinations(size_t output_partition, TableRange left_range, TableRange right_range) {
    left_range.for_every_row_id(_sorted_left_table, [&](RowID& left_row_id) {
      right_range.for_every_row_id(_sorted_right_table, [&](RowID& right_row_id) {
        this->_emit_combination(output_partition, left_row_id, right_row_id);
      });
    });
  }

  /**
  * Emits all combinations of row ids from the left table range and a NULL value on the right side to the join output.
  **/
  void _emit_right_null_combinations(size_t output_partition, TableRange left_range) {
    left_range.for_every_row_id(_sorted_left_table, [&](RowID& left_row_id) {
      this->_emit_combination(output_partition, left_row_id, NULL_ROW_ID);
    });
  }

  /**
  * Emits all combinations of row ids from the right table range and a NULL value on the left side to the join output.
  **/
  void _emit_left_null_combinations(size_t output_partition, TableRange right_range) {
    right_range.for_every_row_id(_sorted_right_table, [&](RowID& right_row_id) {
      this->_emit_combination(output_partition, NULL_ROW_ID, right_row_id);
    });
  }

  /**
  * Determines the length of the run starting at start_index in the values vector.
  * A run is a series of the same value.
  **/
  size_t _run_length(size_t start_index, std::shared_ptr<MaterializedChunk<T>> values) {
    if (start_index >= values->size()) {
      return 0;
    }

    auto& value = (*values)[start_index].value;
    size_t offset = 1;
    while (start_index + offset < values->size() && (*values)[start_index + offset].value == value) {
      ++offset;
    }

    return offset;
  }

  /**
  * Compares two values and creates a comparison result.
  **/
  CompareResult _compare(T left, T right) {
    if (left < right) {
      return CompareResult::Less;
    } else if (left == right) {
      return CompareResult::Equal;
    } else {
      return CompareResult::Greater;
    }
  }

  /**
  * Performs the join on a single partition. Runs of entries with the same value are identified and handled together.
  * This constitutes the merge phase of the join. The output combinations of row ids are determined by _join_runs.
  **/
  void _join_partition(size_t partition_number) {
    _output_pos_lists_left[partition_number] = std::make_shared<PosList>();
    _output_pos_lists_right[partition_number] = std::make_shared<PosList>();

    auto& left_partition = (*_sorted_left_table)[partition_number];
    auto& right_partition = (*_sorted_right_table)[partition_number];

    size_t left_run_start = 0;
    size_t right_run_start = 0;

    auto left_run_end = left_run_start + _run_length(left_run_start, left_partition);
    auto right_run_end = right_run_start + _run_length(right_run_start, right_partition);

    const size_t left_size = left_partition->size();
    const size_t right_size = right_partition->size();

    while (left_run_start < left_size && right_run_start < right_size) {
      auto& left_value = (*left_partition)[left_run_start].value;
      auto& right_value = (*right_partition)[right_run_start].value;

      auto compare_result = _compare(left_value, right_value);

      TableRange left_run(partition_number, left_run_start, left_run_end);
      TableRange right_run(partition_number, right_run_start, right_run_end);
      _join_runs(left_run, right_run, compare_result);

      // Advance to the next run on the smaller side or both if equal
      if (compare_result == CompareResult::Equal) {
        // Advance both runs
        left_run_start = left_run_end;
        right_run_start = right_run_end;
        left_run_end = left_run_start + _run_length(left_run_start, left_partition);
        right_run_end = right_run_start + _run_length(right_run_start, right_partition);
      } else if (compare_result == CompareResult::Less) {
        // Advance the left run
        left_run_start = left_run_end;
        left_run_end = left_run_start + _run_length(left_run_start, left_partition);
      } else {
        // Advance the right run
        right_run_start = right_run_end;
        right_run_end = right_run_start + _run_length(right_run_start, right_partition);
      }
    }

    // There is an edge case in which the last loop run was an "equi hit" and one index
    // reached its maximum size, but one element is potentially still present on the other side.
    // It is important for outer joins and non-equi to include this element.

    // The left side has finished -> add the remaining values of the right side
    if (left_run_start == left_size) {
      if (_op == ScanType::OpEquals && (_mode == JoinMode::Right || _mode == JoinMode::Outer)) {
        _emit_left_null_combinations(partition_number, TableRange(partition_number, right_run_start, right_size));
      }
      if (_op == ScanType::OpGreaterThan || _op == ScanType::OpGreaterThanEquals) {
        auto left_range = TableRange(TablePosition(partition_number + 1, 0).to(_end_of_table(_sorted_left_table)));
        auto right_range = TableRange(partition_number, right_run_start, right_size);
        _emit_combinations(partition_number, left_range, right_range);
      }
    }

    // The right side has finished -> add the remaining values of the left side
    if (right_run_start == right_size) {
      if (_op == ScanType::OpEquals && (_mode == JoinMode::Left || _mode == JoinMode::Outer)) {
        _emit_right_null_combinations(partition_number, TableRange(partition_number, left_run_start, left_size));
      }
      if (_op == ScanType::OpLessThan || _op == ScanType::OpLessThanEquals) {
        auto left_range = TableRange(partition_number, left_run_start, left_size);
        auto right_range = TableRange(TablePosition(partition_number + 1, 0).to(_end_of_table(_sorted_right_table)));
        _emit_combinations(partition_number, left_range, right_range);
      }
    }
  }

    // Join the rest of the unfinished side, which is relevant for outer joins and non-equi joins
    auto right_rest = TableRange(partition_number, right_run_start, right_size);
    auto left_rest = TableRange(partition_number, left_run_start, left_size);
    if (left_run_start < left_size) {
      _join_runs(left_rest, right_rest, CompareResult::Less);
    } else if (right_run_start < right_size) {
      _join_runs(left_rest, right_rest, CompareResult::Greater);
    }
  }

  /**
  * Performs the join on all partitions in parallel.
  **/
  void _perform_join() {
    std::vector<std::shared_ptr<AbstractTask>> jobs;

    // Perform the join on each partition in parallel
    for (size_t partition_number = 0; partition_number < _partition_count; ++partition_number) {
      jobs.push_back(std::make_shared<JobTask>([this, partition_number] {
        this->_join_partition(partition_number);
      }));
      jobs.back()->schedule();
    }

    CurrentScheduler::wait_for_tasks(jobs);

    // Outer join rows for the <, <=, >, >= operators
    // Note: the outer join handling for the equi case is integrated into the main algorithm
    // but that can not be done for the non-equi cases.
    if (_op != ScanType::OpEquals) {
      if (_mode == JoinMode::Left || _mode == JoinMode::Outer) {
        _left_outer_non_equi_join();
      }
      if (_mode == JoinMode::Right || _mode == JoinMode::Outer) {
        _right_outer_non_equi_join();
      }
    }
  }

  /**
  * Concatenates a vector of pos lists into a single new pos list.
  **/
  std::shared_ptr<PosList> _concatenate_pos_lists(std::vector<std::shared_ptr<PosList>>& pos_lists) {
    auto output = std::make_shared<PosList>();

    // Determine the required space
    size_t total_size = 0;
    for (auto& pos_list : pos_lists) {
      total_size += pos_list->size();
    }

    // Move the entries over the output pos list
    output->reserve(total_size);
    for (auto& pos_list : pos_lists) {
      output->insert(output->end(), pos_list->begin(), pos_list->end());
    }

    return output;
  }

  /**
  * Adds the columns from an input table to the output table
  **/
  void _add_output_columns(std::shared_ptr<Table> output_table, std::shared_ptr<const Table> input_table,
                          const std::string& prefix, std::shared_ptr<const PosList> pos_list) {
    auto column_count = input_table->col_count();
    for (ColumnID column_id{0}; column_id < column_count; ++column_id) {
      // Add the column definition
      auto column_name = prefix + input_table->column_name(column_id);
      auto column_type = input_table->column_type(column_id);
      output_table->add_column_definition(column_name, column_type);

      // Add the column data (in the form of a poslist)
      // Check whether the referenced column is already a reference column
      const auto base_column = input_table->get_chunk(ChunkID{0}).get_column(column_id);
      const auto ref_column = std::dynamic_pointer_cast<ReferenceColumn>(base_column);
      if (ref_column) {
        // Create a pos_list referencing the original column instead of the reference column
        auto new_pos_list = _dereference_pos_list(input_table, column_id, pos_list);
        auto new_ref_column = std::make_shared<ReferenceColumn>(ref_column->referenced_table(),
                                                            ref_column->referenced_column_id(), new_pos_list);
        output_table->get_chunk(ChunkID{0}).add_column(new_ref_column);
      } else {
        auto new_ref_column = std::make_shared<ReferenceColumn>(input_table, column_id, pos_list);
        output_table->get_chunk(ChunkID{0}).add_column(new_ref_column);
      }
    }
  }

  /**
  * Turns a pos list that is pointing to reference column entries into a pos list pointing to the original table.
  * This is done because there should not be any reference columns referencing reference columns.
  **/
  std::shared_ptr<PosList> _dereference_pos_list(std::shared_ptr<const Table> input_table, ColumnID column_id,
                                                std::shared_ptr<const PosList> pos_list) {
    // Get all the input pos lists so that we only have to pointer cast the columns once
    auto input_pos_lists = std::vector<std::shared_ptr<const PosList>>();
    for (ChunkID chunk_id{0}; chunk_id < input_table->chunk_count(); ++chunk_id) {
      auto b_column = input_table->get_chunk(chunk_id).get_column(column_id);
      auto r_column = std::dynamic_pointer_cast<ReferenceColumn>(b_column);
      input_pos_lists.push_back(r_column->pos_list());
    }

    // Get the row ids that are referenced
    auto new_pos_list = std::make_shared<PosList>();
    for (const auto& row : *pos_list) {
      new_pos_list->push_back((*input_pos_lists[row.chunk_id])[row.chunk_offset]);
    }

    return new_pos_list;
  }

 public:
  /**
  * Executes the SortMergeJoin operator.
  **/
  std::shared_ptr<const Table> on_execute() {
    auto radix_partitioner = RadixPartitionSort<T>(_sort_merge_join.input_table_left(),
                                                  _sort_merge_join.input_table_right(), *_sort_merge_join._column_names,
                                                  _op == ScanType::OpEquals, _partition_count);
    // Sort and partition the input tables
    auto sort_output = radix_partitioner.execute();
    _sorted_left_table = sort_output.first;
    _sorted_right_table = sort_output.second;
    _end_of_left_table = _end_of_table(_sorted_left_table);
    _end_of_right_table = _end_of_table(_sorted_right_table);

    _perform_join();

    auto output_table = std::make_shared<Table>();

    // merge the pos lists into single pos lists
    auto output_left = _concatenate_pos_lists(_output_pos_lists_left);
    auto output_right = _concatenate_pos_lists(_output_pos_lists_right);

    // Add the columns from both input tables to the output
    _add_output_columns(output_table, _sort_merge_join.input_table_left(),
                        _sort_merge_join._prefix_left, output_left);
    _add_output_columns(output_table, _sort_merge_join.input_table_right(),
                        _sort_merge_join._prefix_right, output_right);

    return output_table;
  }
};

}  // namespace opossum
