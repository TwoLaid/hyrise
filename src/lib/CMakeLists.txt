# Sources and libraries shared among the different builds of the lib
set(
    SOURCES
    all_parameter_variant.cpp
    all_parameter_variant.hpp
    all_type_variant.hpp
    concurrency/commit_context.cpp
    concurrency/commit_context.hpp
    concurrency/transaction_context.cpp
    concurrency/transaction_context.hpp
    concurrency/transaction_manager.cpp
    concurrency/transaction_manager.hpp
    constant_mappings.cpp
    constant_mappings.hpp
    import_export/binary.hpp
    import_export/csv_converter.cpp
    import_export/csv_converter.hpp
    import_export/csv.hpp
    import_export/csv_parser.cpp
    import_export/csv_parser.hpp
    import_export/csv_writer.cpp
    import_export/csv_writer.hpp
    operators/table_scan/base_single_column_table_scan_impl.cpp
    operators/table_scan/base_single_column_table_scan_impl.hpp
    operators/table_scan/base_table_scan_impl.hpp
    operators/table_scan/column_comparison_table_scan_impl.cpp
    operators/table_scan/column_comparison_table_scan_impl.hpp
    operators/table_scan/is_null_table_scan_impl.cpp
    operators/table_scan/is_null_table_scan_impl.hpp
    operators/table_scan/like_table_scan_impl.cpp
    operators/table_scan/like_table_scan_impl.hpp
    operators/table_scan/single_column_table_scan_impl.cpp
    operators/table_scan/single_column_table_scan_impl.hpp
    operators/abstract_join_operator.cpp
    operators/abstract_join_operator.hpp
    operators/abstract_operator.cpp
    operators/abstract_operator.hpp
    operators/abstract_read_only_operator.cpp
    operators/abstract_read_only_operator.hpp
    operators/abstract_read_write_operator.cpp
    operators/abstract_read_write_operator.hpp
    operators/aggregate.cpp
    operators/aggregate.hpp
    operators/delete.cpp
    operators/delete.hpp
    operators/difference.cpp
    operators/difference.hpp
    operators/export_binary.cpp
    operators/export_binary.hpp
    operators/export_csv.cpp
    operators/export_csv.hpp
    operators/get_table.cpp
    operators/get_table.hpp
    operators/import_binary.cpp
    operators/import_binary.hpp
    operators/import_csv.cpp
    operators/import_csv.hpp
    operators/index_column_scan.cpp
    operators/index_column_scan.hpp
    operators/insert.cpp
    operators/insert.hpp
    operators/join_hash.cpp
    operators/join_hash.hpp
    operators/join_sort_merge.cpp
    operators/join_sort_merge.hpp
    operators/join_nested_loop_a.cpp
    operators/join_nested_loop_a.hpp
    operators/join_nested_loop_b.cpp
    operators/join_nested_loop_b.hpp
    operators/limit.cpp
    operators/limit.hpp
    operators/maintenance/show_columns.cpp
    operators/maintenance/show_columns.hpp
    operators/maintenance/show_tables.cpp
    operators/maintenance/show_tables.hpp
    operators/print.cpp
    operators/print.hpp
    operators/product.cpp
    operators/product.hpp
    operators/projection.cpp
    operators/projection.hpp
    operators/sort.cpp
    operators/sort.hpp
    operators/table_scan.cpp
    operators/table_scan.hpp
    operators/table_wrapper.cpp
    operators/table_wrapper.hpp
    operators/union_all.cpp
    operators/union_all.hpp
    operators/union_positions.cpp
    operators/union_positions.hpp
    operators/update.cpp
    operators/update.hpp
    operators/validate.cpp
    operators/validate.hpp
    optimizer/abstract_syntax_tree/abstract_ast_node.cpp
    optimizer/abstract_syntax_tree/abstract_ast_node.hpp
    optimizer/abstract_syntax_tree/abstract_non_optimizable_ast_node.cpp
    optimizer/abstract_syntax_tree/abstract_non_optimizable_ast_node.hpp
    optimizer/abstract_syntax_tree/aggregate_node.cpp
    optimizer/abstract_syntax_tree/aggregate_node.hpp
    optimizer/abstract_syntax_tree/ast_root_node.cpp
    optimizer/abstract_syntax_tree/ast_root_node.hpp
    optimizer/abstract_syntax_tree/ast_to_operator_translator.hpp
    optimizer/abstract_syntax_tree/ast_to_operator_translator.cpp
    optimizer/abstract_syntax_tree/delete_node.cpp
    optimizer/abstract_syntax_tree/delete_node.hpp
    optimizer/abstract_syntax_tree/dummy_table_node.cpp
    optimizer/abstract_syntax_tree/dummy_table_node.hpp
    optimizer/abstract_syntax_tree/join_node.cpp
    optimizer/abstract_syntax_tree/join_node.hpp
    optimizer/abstract_syntax_tree/insert_node.cpp
    optimizer/abstract_syntax_tree/insert_node.hpp
    optimizer/abstract_syntax_tree/limit_node.cpp
    optimizer/abstract_syntax_tree/limit_node.hpp
    optimizer/abstract_syntax_tree/mock_node.cpp
    optimizer/abstract_syntax_tree/mock_node.hpp
    optimizer/abstract_syntax_tree/predicate_node.cpp
    optimizer/abstract_syntax_tree/predicate_node.hpp
    optimizer/abstract_syntax_tree/projection_node.cpp
    optimizer/abstract_syntax_tree/projection_node.hpp
    optimizer/abstract_syntax_tree/show_columns_node.cpp
    optimizer/abstract_syntax_tree/show_columns_node.hpp
    optimizer/abstract_syntax_tree/show_tables_node.cpp
    optimizer/abstract_syntax_tree/show_tables_node.hpp
    optimizer/abstract_syntax_tree/sort_node.cpp
    optimizer/abstract_syntax_tree/sort_node.hpp
    optimizer/abstract_syntax_tree/stored_table_node.cpp
    optimizer/abstract_syntax_tree/stored_table_node.hpp
    optimizer/abstract_syntax_tree/union_node.cpp
    optimizer/abstract_syntax_tree/union_node.hpp
    optimizer/abstract_syntax_tree/update_node.cpp
    optimizer/abstract_syntax_tree/update_node.hpp
    optimizer/abstract_syntax_tree/validate_node.cpp
    optimizer/abstract_syntax_tree/validate_node.hpp
    optimizer/base_column_statistics.cpp
    optimizer/base_column_statistics.hpp
    optimizer/column_statistics.cpp
    optimizer/column_statistics.hpp
    optimizer/expression.cpp
    optimizer/expression.hpp
    optimizer/optimizer.cpp
    optimizer/optimizer.hpp
    optimizer/strategy/abstract_rule.cpp
    optimizer/strategy/abstract_rule.hpp
    optimizer/strategy/join_detection_rule.cpp
    optimizer/strategy/join_detection_rule.hpp
    optimizer/strategy/predicate_reordering_rule.cpp
    optimizer/strategy/predicate_reordering_rule.hpp
    optimizer/table_statistics.cpp
    optimizer/table_statistics.hpp
    planviz/ast_visualizer.cpp
    planviz/ast_visualizer.hpp
    planviz/sql_query_plan_visualizer.cpp
    planviz/sql_query_plan_visualizer.hpp
    resolve_type.hpp
    scheduler/abstract_scheduler.cpp
    scheduler/abstract_scheduler.hpp
    scheduler/abstract_task.cpp
    scheduler/abstract_task.hpp
    scheduler/current_scheduler.cpp
    scheduler/current_scheduler.hpp
    scheduler/job_task.cpp
    scheduler/job_task.hpp
    scheduler/node_queue_scheduler.cpp
    scheduler/node_queue_scheduler.hpp
    scheduler/operator_task.cpp
    scheduler/operator_task.hpp
    scheduler/processing_unit.cpp
    scheduler/processing_unit.hpp
    scheduler/task_queue.cpp
    scheduler/task_queue.hpp
    scheduler/topology.cpp
    scheduler/topology.hpp
    scheduler/worker.cpp
    scheduler/worker.hpp
    sql/lru_cache.hpp
    sql/sql_planner.cpp
    sql/sql_planner.hpp
    sql/sql_expression_translator.cpp
    sql/sql_expression_translator.hpp
    sql/sql_query_cache.hpp
    sql/sql_query_operator.cpp
    sql/sql_query_operator.hpp
    sql/sql_query_plan.cpp
    sql/sql_query_plan.hpp
    sql/sql_result_operator.cpp
    sql/sql_result_operator.hpp
    sql/sql_to_ast_translator.cpp
    sql/sql_to_ast_translator.hpp
    storage/base_attribute_vector.hpp
    storage/base_column.hpp
    storage/chunk.cpp
    storage/chunk.hpp
    storage/column_visitable.hpp
    storage/copyable_atomic.hpp
    storage/dictionary_column.cpp
    storage/dictionary_column.hpp
    storage/dictionary_compression.cpp
    storage/dictionary_compression.hpp
    storage/fitted_attribute_vector.hpp
    storage/index/adaptive_radix_tree/adaptive_radix_tree_index.cpp
    storage/index/adaptive_radix_tree/adaptive_radix_tree_index.hpp
    storage/index/adaptive_radix_tree/adaptive_radix_tree_nodes.cpp
    storage/index/adaptive_radix_tree/adaptive_radix_tree_nodes.hpp
    storage/index/base_index.cpp
    storage/index/base_index.hpp
    storage/index/group_key/composite_group_key_index.cpp
    storage/index/group_key/composite_group_key_index.hpp
    storage/index/group_key/group_key_index.cpp
    storage/index/group_key/group_key_index.hpp
    storage/index/group_key/variable_length_key_base.cpp
    storage/index/group_key/variable_length_key_base.hpp
    storage/index/group_key/variable_length_key.cpp
    storage/index/group_key/variable_length_key.hpp
    storage/index/group_key/variable_length_key_proxy.cpp
    storage/index/group_key/variable_length_key_proxy.hpp
    storage/index/group_key/variable_length_key_store.cpp
    storage/index/group_key/variable_length_key_store.hpp
    storage/iterables/attribute_vector_iterable.hpp
    storage/iterables/iterables.hpp
    storage/iterables/base_iterators.hpp
    storage/iterables/chunk_offset_mapping.cpp
    storage/iterables/chunk_offset_mapping.hpp
    storage/iterables/column_value.hpp
    storage/iterables/constant_value_iterable.hpp
    storage/iterables/create_iterable_from_column.hpp
    storage/iterables/dictionary_column_iterable.hpp
    storage/iterables/null_value_vector_iterable.hpp
    storage/iterables/reference_column_iterable.hpp
    storage/iterables/value_column_iterable.hpp
    storage/reference_column.cpp
    storage/reference_column.hpp
    storage/scoped_locking_ptr.hpp
    storage/storage_manager.cpp
    storage/storage_manager.hpp
    storage/table.cpp
    storage/table.hpp
    storage/base_dictionary_column.hpp
    storage/base_value_column.hpp
    storage/value_column.cpp
    storage/value_column.hpp
    tasks/chunk_compression_task.cpp
    tasks/chunk_compression_task.hpp
    type_cast.hpp
    type_comparison.hpp
    types.hpp
    uid_allocator.hpp
    utils/assert.hpp
    utils/boost_default_memory_resource.cpp
    utils/cuckoo_hashtable.hpp
    utils/load_table.cpp
    utils/load_table.hpp
    utils/murmur_hash.cpp
    utils/murmur_hash.hpp
    utils/numa_memory_resource.cpp
    utils/numa_memory_resource.hpp
    utils/performance_warning.cpp
    utils/performance_warning.hpp
)

set(
    LIBRARIES
    pthread
    sqlparser
    ${TBB_LIBRARY}
)

if (NOT DISABLE_NUMA_SUPPORT AND ${NUMA_FOUND})
    set(LIBRARIES ${LIBRARIES} ${NUMA_LIBRARY} hpinuma_msource_s)
endif()

# Configure the regular hyrise library used for tests/server/playground...
add_library(hyrise STATIC ${SOURCES})
target_link_libraries(hyrise ${LIBRARIES})

add_sanitizer_library(hyriseSanitizersLib "${SOURCES}" "${LIBRARIES}")
add_coverage_library(hyriseCoverageLib "${SOURCES}" "${LIBRARIES}")
