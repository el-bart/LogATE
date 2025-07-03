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
  BUT_DEFINE_EXCEPTION(InvalidKeyPath, Error, "invalid key path");

  enum SourceFormat
  {
    Raw,        // use raw format of the key (i.e. as it is defined; just converted to the string)
    ISO8601_ns, // ISO8601 timestamp with nanoseconds precision (this is the format used internally)
    UNIX,       // UNIX timestamp (in seconds)
    UNIX_ms,    // UNIX timestamp in miliseconds
    UNIX_us,    // UNIX timestamp in microseconds
    UNIX_ns     // UNIX timestamp in nanoseconds
  };

  KeyExtractor(Path path, SourceFormat const sourceFormat);

  KeyExtractor(KeyExtractor&&) = default;
  KeyExtractor& operator=(KeyExtractor&&) = default;
  KeyExtractor(KeyExtractor const&) = default;
  KeyExtractor& operator=(KeyExtractor const&) = default;

  std::string extract(nlohmann::json const& in) const; // TODO: legacy - remove
  std::string operator()(nlohmann::json const& in) const
  {
    return extract(in);
  }

  auto const& path() const { return path_; }

private:
  Path path_;
  SourceFormat const sourceFormat_;
};


using KeyExtractorShNN = But::NotNull<std::shared_ptr<KeyExtractor>>;

}
