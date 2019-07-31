#include "LogATE/Log.hpp"
#include "LogATE/Utils/value2str.hpp"
#include "LogATE/Utils/zeroPaddedString.hpp"
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


namespace
{
auto getKey(nlohmann::json const& json, Tree::Path const& keyPath, const SequenceNumber sn)
{
  auto* node = &json;
  BUT_ASSERT( keyPath.absolute() );
  BUT_ASSERT( not keyPath.empty() );
  for(auto pit=keyPath.begin()+1; pit!=keyPath.end(); ++pit)
  {
    auto it = node->find(*pit);
    if( it == node->end() )
      return Log::Key{"<< key not found >>", sn};
    node = &*it;
  }
  auto opt = Utils::value2str(*node);
  if(not opt)
    return Log::Key{"<< key not found >>", sn};
  return Log::Key{ std::move(*opt), sn };
}
}

AnnotatedLog::AnnotatedLog(std::string str, Tree::Path const& keyPath):
  json_( nlohmann::json::parse(str) ),
  log_{ Log::DirectInitTag{}, getKey( json_, keyPath, SequenceNumber::next() ), std::move(str) }
{ }

}
