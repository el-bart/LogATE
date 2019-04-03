#include "LogATE/Log.hpp"

namespace LogATE
{

Log json2log(std::string const& in)
{
  return makeLog( nlohmann::json::parse(in) );
}

Log makeLog(nlohmann::json json)
{
  return Log{ SequenceNumber::next(), But::makeSharedNN<const nlohmann::json>( std::move(json) ) };
}

}
