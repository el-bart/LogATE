#include "LogATE/Log.hpp"

namespace LogATE
{

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


Log::Log(DirectInitTag&&, SequenceNumber sn, std::string in):
  sn_{sn},
  str_{ But::makeSharedNN<const std::string>( std::move(in) ) }
{
  BUT_ASSERT( not nlohmann::json(*str_).dump().empty() && "given string is NOT a JSON..." );
}


AnnotatedLog::AnnotatedLog(Log log):
  log_{ std::move(log) },
  json_( log_.json() )
{ }

AnnotatedLog::AnnotatedLog(nlohmann::json in):
  log_{in},
  json_{ std::move(in) }
{ }

AnnotatedLog::AnnotatedLog(const SequenceNumber sn, nlohmann::json in):
  log_{sn, in},
  json_{ std::move(in) }
{ }

}
