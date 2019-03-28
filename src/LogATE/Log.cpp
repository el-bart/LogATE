#include "LogATE/Log.hpp"

namespace LogATE
{

Log json2log(std::string const& in)
{
  auto json = nlohmann::json::parse(in);
  return Log{ {}, But::makeSharedNN<const nlohmann::json>(std::move(json)) };
}

}
