#include "import_csv.hpp"

#include <fstream>
#include <memory>
#include <optional>
#include <string>

#include "import_export/csv_parser.hpp"
#include "storage/storage_manager.hpp"
#include "utils/assert.hpp"

namespace opossum {

ImportCsv::ImportCsv(const std::string& filename, const std::optional<std::string> tablename)
    : _filename(filename), _tablename(tablename), _custom_meta(false) {}

ImportCsv::ImportCsv(const std::string& filename, const CsvMeta& meta, const std::optional<std::string> tablename)
    : _filename(filename), _tablename(tablename), _meta(meta), _custom_meta(true) {}

const std::string ImportCsv::name() const { return "ImportCSV"; }

std::shared_ptr<const Table> ImportCsv::_on_execute() {
  if (_tablename && StorageManager::get().has_table(*_tablename)) {
    return StorageManager::get().get_table(*_tablename);
  }

  // Check if file exists before giving it to the parser
  std::ifstream file(_filename);
  Assert(file.is_open(), "ImportCsv: Could not find file " + _filename);
  file.close();

  std::shared_ptr<Table> table;
  CsvParser parser;
  if (_custom_meta) {
    table = parser.parse(_filename, _meta);
  } else {
    table = parser.parse(_filename);
  }

  if (_tablename) {
    StorageManager::get().add_table(*_tablename, table);
  }

  return table;
}

}  // namespace opossum
