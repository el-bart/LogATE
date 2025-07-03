#include "LogATE/Log.hpp"
#include "LogATE/Utils/value2str.hpp"
#include <iomanip>
#include <sstream>

namespace LogATE
{

Log::Log(Key key, std::string const& in):
  Log{ std::move(key), nlohmann::json::parse(in) }
{ }


namespace
{
auto minimalString(nlohmann::json const& in)
{
  auto str = in.dump();
  str.shrink_to_fit();
  return str;
}
}

Log::Log(Key key, nlohmann::json const& in):
  Log{ DirectInitTag{}, std::move(key), minimalString(in) }
{ }


Log::Log(DirectInitTag&&, Key&& key, std::string&& in):
  str_{ But::makeSharedNN<const std::string>( std::move(in) ) },
  key_{ std::move(key) }
{
  BUT_ASSERT( not nlohmann::json(*str_).dump().empty() && "given string is NOT a JSON..." );
}


AnnotatedLog::AnnotatedLog(Log log):
  json_( log.json() ),
  log_{ std::move(log) }
{ }


AnnotatedLog::AnnotatedLog(std::string str, Tree::KeyExtractor const& keyExtractor):
  json_( nlohmann::json::parse(str) ),
  log_{ Log::DirectInitTag{}, Log::Key{ keyExtractor(json_), SequenceNumber::next() }, std::move(str) }
{ }

}
