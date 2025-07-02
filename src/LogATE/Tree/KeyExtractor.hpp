#pragma once
#include <string>
#include <nlohmann/json.hpp>
//#include <But/Exception.hpp>
//#include <But/assert.hpp>
#include "LogATE/Tree/Path.hpp"

namespace LogATE::Tree
{

struct KeyExtractor final
{
  enum Format
  {
    ISO8601_ns, // ISO8601 timestamp with nanoseconds precision
    UNIX,       // UNIX timestamp (in seconds)
    UNIX_ms,    // UNIX timestamp in miliseconds
    UNIX_us,    // UNIX timestamp in microseconds
    UNIX_ns     // UNIX timestamp in nanoseconds
  };

  KeyExtractor(Path path, Format const format):
    path_{ std::move(path) },
    format_{format}
  { }

  std::string getKey(nlohmann::json const& in) const;

private:
  Path const path_;
  Format const format_;
};

}
