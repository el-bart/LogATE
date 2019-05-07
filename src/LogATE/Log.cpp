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


Log::Log(const SequenceNumber sn, nlohmann::json const& in):
  sn_{sn},
  str_{ But::makeSharedNN<const std::string>( in.dump() ) }
{ }

}
