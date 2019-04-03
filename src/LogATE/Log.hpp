#pragma once
#include "SequenceNumber.hpp"
#include <nlohmann/json.hpp>
#include <But/NotNull.hpp>
#include <string>

namespace LogATE
{

struct Log
{
  SequenceNumber sn_;
  But::NotNullShared<const nlohmann::json> log_;
};

Log json2log(std::string const& in);
Log makeLog(nlohmann::json json);

}
