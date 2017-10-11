#include "chunk_metrics_collection_task.hpp"

#include <memory>
#include <string>
#include <vector>

#include "storage/chunk.hpp"
#include "storage/storage_manager.hpp"
#include "storage/table.hpp"

namespace opossum {

ChunkMetricsCollectionTask::ChunkMetricsCollectionTask() {}

void ChunkMetricsCollectionTask::_on_execute() {
  const auto& table_names = StorageManager::get().table_names();
  for (const auto& table_name : table_names) {
    const auto& table = StorageManager::get().get_table(table_name);
    for (ChunkID i = ChunkID(0); i < table->chunk_count(); i++) {
      const auto& _chunk = table->get_chunk(i);
      if (const auto access_counter = _chunk.access_counter()) {
        access_counter->process();
      }
    }
  }
}

}  // namespace opossum