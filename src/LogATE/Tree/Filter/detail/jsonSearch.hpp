#pragma once
#include "LogATE/Log.hpp"
#include "LogATE/Path.hpp"
#include <regex>

namespace LogATE::Tree::Filter::detail
{

bool matchesKey(Log const& log, std::regex const& re);
bool matchesValue(Log const& log, std::regex const& re);

}
