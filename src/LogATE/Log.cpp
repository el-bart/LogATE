#include "LogATE/Log.hpp"
#include "LogATE/Utils/value2str.hpp"
#include <iomanip>
#include <sstream>

namespace LogATE
{

namespace
{
template<typename S>
auto copyData(S const& s)
{
  auto nn = But::makeSharedNN<char[]>( s.size() );
  strncpy( nn.get(), s.data(), s.size() );
  return nn;
}
}

Log::Key::Key(std::string_view const& value):
  size_{ value.size() },
  value_{ copyData(value) }
{ }


Log::Key::Key(std::string const& value):
  size_{ value.size() },
  value_{ copyData(value) }
{ }


Log Log::acceptRawString(std::string in)
{
  return acceptRawString( SequenceNumber::next(), std::move(in) );
}


Log Log::acceptRawString(const SequenceNumber sn, std::string in)
{
  return Log{ DirectInitTag{}, sn, std::move(in) };
}

Log::Log(std::string const& in):
  Log{ SequenceNumber::next(), nlohmann::json::parse(in) }
{ }


Log::Log(nlohmann::json const& in):
  Log{ SequenceNumber::next(), in }
{ }


Log::Log(const SequenceNumber sn, std::string const& in):
  Log{ sn, nlohmann::json::parse(in) }
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

Log::Log(const SequenceNumber sn, nlohmann::json const& in):
  Log{ DirectInitTag{}, sn, minimalString(in) }
{ }


namespace
{
auto toPaddedString(const SequenceNumber sn)
{
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(20) << sn.value_;
  return ss.str();
}
}

Log::Log(DirectInitTag&& dit, SequenceNumber sn, std::string in):
  Log{ std::move(dit), sn, std::move(in), Key{ toPaddedString(sn) } }
{ }

Log::Log(DirectInitTag&&, SequenceNumber sn, std::string in, Key key):
  sn_{sn},
  str_{ But::makeSharedNN<const std::string>( std::move(in) ) },
  key_{ std::move(key) }
{
  BUT_ASSERT( not nlohmann::json(*str_).dump().empty() && "given string is NOT a JSON..." );
}


AnnotatedLog::AnnotatedLog(std::string str):
  log_{ Log::DirectInitTag{}, SequenceNumber::next(), std::move(str) },
  json_( log_.json() )
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
      return Log::Key{ "<< key not found >>" + std::to_string(sn.value_) };
    node = &*it;
  }
  auto opt = Utils::value2str(*node);
  if(not opt)
    return Log::Key{ "<< key not found >>" + std::to_string(sn.value_) };
  return Log::Key{ *opt + " / " + std::to_string(sn.value_) };
}
}

AnnotatedLog::AnnotatedLog(std::string str, Tree::Path const& keyPath):
  log_{ Log::DirectInitTag{}, SequenceNumber::next(), std::move(str), Log::Key{""} },
  json_( log_.json() )
{
  log_.key_ = getKey( json_, keyPath, log_.sequenceNumber() );
}

AnnotatedLog::AnnotatedLog(Log log):
  log_{ std::move(log) },
  json_( log_.json() )
{ }

AnnotatedLog::AnnotatedLog(nlohmann::json in):
  log_{in},
  json_( std::move(in) )
{ }

AnnotatedLog::AnnotatedLog(const SequenceNumber sn, nlohmann::json in):
  log_{sn, in},
  json_( std::move(in) )
{ }

}
