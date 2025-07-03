#pragma once
#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include <But/NotNull.hpp>
#include <But/Exception.hpp>
#include "LogATE/Tree/Path.hpp"

namespace LogATE::Tree
{

struct KeyExtractor final
{
  BUT_DEFINE_EXCEPTION(Error, But::Exception, "KeyExtractor error");

  enum SourceFormat
  {
    Raw,        // use raw format of the key (i.e. as it is defined; just converted to the string)
    ISO8601_ns, // ISO8601 timestamp with nanoseconds precision (this is the format used internally)
    UNIX,       // UNIX timestamp (in seconds)
    UNIX_ms,    // UNIX timestamp in miliseconds
    UNIX_us,    // UNIX timestamp in microseconds
    UNIX_ns     // UNIX timestamp in nanoseconds
  };

  KeyExtractor(Path path, SourceFormat const sourceFormat):
    path_{ std::move(path) },
    sourceFormat_{sourceFormat}
  { }

  std::string extract(nlohmann::json const& in) const;

private:
  Path const path_;
  SourceFormat const sourceFormat_;
};


using KeyExtractorShPtrNN = But::NotNull<std::shared_ptr<KeyExtractor>>;

}
