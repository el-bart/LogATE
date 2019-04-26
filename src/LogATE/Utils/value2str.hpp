#pragma once
#include <nlohmann/json.hpp>
#include <But/Optional.hpp>
#include <string>

namespace LogATE::Utils
{
But::Optional<std::string> value2str(nlohmann::json const& node);
}
