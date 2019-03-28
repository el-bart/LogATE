#include "LogATE/Log.hpp"
#include <iostream>

namespace LogATE
{

Log json2log(std::string const& in)
{
  auto json = nlohmann::json::parse(in);
  return Log{ SequenceNumber::next(), But::makeSharedNN<const nlohmann::json>(std::move(json)) };
}

std::ostream& operator<<(std::ostream& os, Log const& log)
{
  return os << "< " << log.sn_ << " ; " << *log.log_ << " >";
}

}
