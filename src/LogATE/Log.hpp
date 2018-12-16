#pragma once
#include <nlohmann/json.hpp>
#include <But/NotNull.hpp>
#include <string>

namespace LogATE
{

using Log = But::NotNullShared<nlohmann::json>;

Log parseLog(std::string const& in);

}
