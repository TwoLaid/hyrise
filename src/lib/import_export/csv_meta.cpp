#include "csv_meta.hpp"

#include <fstream>

#include "utils/assert.hpp"

namespace opossum {

CsvMeta process_csv_meta_file(const std::string& filename) {
  std::ifstream metafile{filename};
  Assert(metafile.good(), "Meta file does not exist: " + filename);
  nlohmann::json meta_json;
  metafile >> meta_json;
  CsvMeta meta = meta_json;
  return meta;
}

void assign_if_exists(char &value, const nlohmann::json& json_object, const std::string &key) {
  if (json_object.find(key) != json_object.end()) {
    std::string character = json_object.at(key);
    Assert(character.size() == 1, "CSV meta file config: Character specifications can only be a single character.");
    value = character[0];
  }
}

void assign_if_exists(bool &value, const nlohmann::json& json_object, const std::string &key) {
  if (json_object.find(key) != json_object.end()) {
    value = json_object.at(key);
  }
}

void from_json(const nlohmann::json& json, CsvMeta& meta) {
  meta.chunk_size = json.at("chunk_size");

  // Apply only parts of the ParseConfig that are provided, use default values otherwise
  ParseConfig config{};
  if (json.find("config") != json.end()) {
    Assert(json.at("columns").is_object(), "CSV meta file,\"Config\" field has to be a json object.");
    nlohmann::json config_json = json.at("config");
    assign_if_exists(config.delimiter, config_json, "delimiter");
    assign_if_exists(config.separator, config_json, "separator");
    assign_if_exists(config.quote, config_json, "quote");
    assign_if_exists(config.escape, config_json, "escape");
    assign_if_exists(config.delimiter_escape, config_json, "delimiter_escape");
    assign_if_exists(config.reject_quoted_nonstrings, config_json, "reject_quoted_nonstrings");
    assign_if_exists(config.rfc_mode, config_json, "rfc_mode");
  }

  if (json.find("columns") != json.end()) {
    Assert(json.at("columns").is_array(), "CSV meta file,\"Columns\" field has to be a json array.");
    std::vector<ColumnMeta> columns;
    for (auto column : json.at("columns")) {
      ColumnMeta column_meta;
      column_meta.name = column.at("name");
      column_meta.type = column.at("type");
      assign_if_exists(column_meta.nullable, column, "nullable");
      columns.push_back(column_meta);
    }
    meta.columns = columns;
  }

  meta.config = config;
}

void to_json(nlohmann::json& json, const CsvMeta& meta) {
  nlohmann::json config = nlohmann::json {
    {"delimiter", meta.config.delimiter},
    {"separator", meta.config.separator},
    {"quote", meta.config.quote},
    {"escape", meta.config.escape},
    {"delimiter_escape", meta.config.delimiter_escape},
    {"reject_quoted_nonstrings", meta.config.reject_quoted_nonstrings},
    {"rfc_mode", meta.config.rfc_mode}
  };

  nlohmann::json columns = "[]"_json;
  for (auto column_meta : meta.columns) {
    nlohmann::json column_meta_json = nlohmann::json {
      {"name", column_meta.name},
      {"type", column_meta.type},
      {"nullable", column_meta.nullable}
    };
    columns.push_back(column_meta_json);
  }

  json = nlohmann::json {
    {"chunk_size", meta.chunk_size},
    {"config", config},
    {"columns", columns}
  };
}

}  // namespace opossum
