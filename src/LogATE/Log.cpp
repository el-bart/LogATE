#include "LogATE/Log.hpp"

namespace LogATE
{

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
  sn_{sn},
  str_{ But::makeSharedNN<const std::string>( minimalString(in) ) }
{ }


AnnotatedLog::AnnotatedLog(Log const& log):
  log_{log},
  json_( log_.json() )
{ }

}
