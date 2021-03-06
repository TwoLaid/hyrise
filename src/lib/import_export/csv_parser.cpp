#include "csv_parser.hpp"

#include <fstream>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "import_export/csv_converter.hpp"
#include "resolve_type.hpp"
#include "scheduler/job_task.hpp"
#include "storage/table.hpp"
#include "utils/assert.hpp"
#include "utils/load_table.hpp"

namespace opossum {

CsvParser::CsvParser(const CsvConfig& csv_config) : _csv_config(csv_config) {}

std::shared_ptr<Table> CsvParser::parse(const std::string& filename) {
  const auto table = _process_meta_file(filename + _csv_config.meta_file_extension);

  std::ifstream csvfile{filename};
  std::string content{std::istreambuf_iterator<char>(csvfile), {}};

  // return empty table if input file is empty
  if (!csvfile) return table;

  // make sure content ends with a delimiter for better row processing later
  if (content.back() != _csv_config.delimiter) content.push_back(_csv_config.delimiter);

  std::string_view content_view{content.c_str(), content.size()};

  // Save chunks in list to avoid memory relocation
  std::list<Chunk> chunks;
  std::vector<std::shared_ptr<JobTask>> tasks;
  std::vector<size_t> field_ends;
  while (_find_fields_in_chunk(content_view, *table.get(), field_ends)) {
    // create empty chunk
    chunks.emplace_back(ChunkUseMvcc::Yes);
    auto& chunk = chunks.back();

    // Only pass the part of the string that is actually needed to the parsing task
    std::string_view relevant_content = content_view.substr(0, field_ends.back());

    // Remove processed part of the csv content
    content_view = content_view.substr(field_ends.back() + 1);

    // create and start parsing task to fill chunk
    tasks.emplace_back(std::make_shared<JobTask>([this, relevant_content, field_ends, &table, &chunk]() {
      _parse_into_chunk(relevant_content, field_ends, *table, chunk);
    }));
    tasks.back()->schedule();
  }

  for (auto& task : tasks) {
    task->join();
  }

  for (auto& chunk : chunks) {
    table->emplace_chunk(std::move(chunk));
  }

  return table;
}

std::shared_ptr<Table> CsvParser::_process_meta_file(const std::string& filename) {
  const char delimiter = '\n';
  const char separator = ',';

  std::ifstream metafile{filename};
  std::string content{std::istreambuf_iterator<char>(metafile), {}};

  // make sure content ends with '\n' for better row processing later
  if (content.back() != delimiter) content.push_back(delimiter);

  // skip header
  content.erase(0, content.find(delimiter) + 1);

  // skip next two fields
  content.erase(0, content.find(separator) + 1);
  content.erase(0, content.find(separator) + 1);

  // read chunk size
  auto pos = content.find(delimiter);
  const size_t chunk_size{std::stoul(content.substr(0, pos))};
  content.erase(0, pos + 1);

  const auto table = std::make_shared<Table>(chunk_size);

  // read column info
  while ((pos = content.find(delimiter)) != std::string::npos) {
    auto row = content.substr(0, pos);

    // remove property type
    auto property_type_pos = row.find(separator);
    row.erase(0, property_type_pos + 1);

    // read column name
    auto row_pos = row.find(separator);
    auto column_name = row.substr(0, row_pos);
    BaseCsvConverter::unescape(column_name);
    row.erase(0, row_pos + 1);

    // read column type
    row_pos = row.find(delimiter);
    auto column_type = row.substr(0, row_pos);
    BaseCsvConverter::unescape(column_type);
    auto type_nullable = _split<std::string>(column_type, '_');
    column_type = type_nullable[0];

    auto is_nullable = type_nullable.size() > 1 && boost::to_lower_copy(type_nullable[1]) == CsvConfig::NULL_STRING;

    content.erase(0, pos + 1);

    table->add_column_definition(column_name, column_type, is_nullable);
  }

  return table;
}

bool CsvParser::_find_fields_in_chunk(std::string_view csv_content, const Table& table,
                                      std::vector<size_t>& field_ends) {
  field_ends.clear();
  if (csv_content.empty()) {
    return false;
  }

  std::string search_for{_csv_config.separator, _csv_config.delimiter, _csv_config.quote};

  size_t pos, from = 0;
  unsigned int rows = 0, field_count = 1;
  bool in_quotes = false;
  while (rows < table.chunk_size() || 0 == table.chunk_size()) {
    // Find either of row separator, column delimiter, quote identifier
    pos = csv_content.find_first_of(search_for, from);
    if (std::string::npos == pos) {
      break;
    }
    from = pos + 1;
    const char elem = csv_content.at(pos);

    // Make sure to "toggle" in_quotes ONLY if the quotes are not part of the string (i.e. escaped)
    if (elem == _csv_config.quote) {
      bool quote_is_escaped = false;
      if (_csv_config.quote != _csv_config.escape) {
        quote_is_escaped = pos != 0 && csv_content.at(pos - 1) == _csv_config.escape;
      }
      if (!quote_is_escaped) {
        in_quotes = !in_quotes;
      }
    }

    // Determine if delimiter marks end of row or is part of the (string) value
    if (elem == _csv_config.delimiter && !in_quotes) {
      Assert(field_count == table.column_count(), "Number of CSV fields does not match number of columns.");
      ++rows;
      field_count = 0;
    }

    // Determine if separator marks end of field or is part of the (string) value
    if (in_quotes || elem == _csv_config.quote) {
      continue;
    }

    ++field_count;
    field_ends.push_back(pos);
  }

  return true;
}

void CsvParser::_parse_into_chunk(std::string_view csv_chunk, const std::vector<size_t>& field_ends, const Table& table,
                                  Chunk& chunk) {
  // For each csv column create a CsvConverter which builds up a ValueColumn
  const auto column_count = table.column_count();
  const auto row_count = field_ends.size() / column_count;
  std::vector<std::unique_ptr<BaseCsvConverter>> converters;

  for (ColumnID column_id{0}; column_id < column_count; ++column_id) {
    const auto is_nullable = table.column_is_nullable(column_id);
    const auto column_type = table.column_type(column_id);

    converters.emplace_back(
        make_unique_by_column_type<BaseCsvConverter, CsvConverter>(column_type, row_count, _csv_config, is_nullable));
  }

  size_t start = 0;
  for (size_t row_id = 0; row_id < row_count; ++row_id) {
    for (ColumnID column_id{0}; column_id < column_count; ++column_id) {
      const auto end = field_ends.at(row_id * column_count + column_id);
      auto field = std::string{csv_chunk.substr(start, end - start)};
      start = end + 1;

      if (!_csv_config.rfc_mode) {
        // CSV fields not following RFC 4810 might need some preprocessing
        _sanitize_field(field);
      }

      try {
        converters[column_id]->insert(field, row_id);
      } catch (const std::exception& exception) {
        std::cerr << "Exception while parsing CSV, row " << row_id << ", column " << column_id << "." << std::endl;
        throw;
      }
    }
  }

  // Transform the field_offsets to columns and add columns to chunk.
  for (auto& converter : converters) {
    chunk.add_column(converter->finish());
  }
}

void CsvParser::_sanitize_field(std::string& field) {
  const std::string linebreak(1, _csv_config.delimiter);
  const std::string escaped_linebreak =
      std::string(1, _csv_config.delimiter_escape) + std::string(1, _csv_config.delimiter);

  std::string::size_type pos = 0;
  while ((pos = field.find(escaped_linebreak, pos)) != std::string::npos) {
    field.replace(pos, escaped_linebreak.size(), linebreak);
    pos += linebreak.size();
  }
}

}  // namespace opossum
