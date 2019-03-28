#pragma once
#include "SequenceNumber.hpp"
#include <nlohmann/json.hpp>
#include <But/NotNull.hpp>
#include <string>
#include <iosfwd>

namespace LogATE
{

struct Log
{
  SequenceNumber sn_;
  But::NotNullShared<const nlohmann::json> log_;
};

Log json2log(std::string const& in);

std::ostream& operator<<(std::ostream& os, Log const& log);

}
