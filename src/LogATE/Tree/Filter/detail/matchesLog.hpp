#pragma once
#include "LogATE/Log.hpp"
#include "LogATE/Tree/Path.hpp"
#include <vector>
#include <regex>

namespace LogATE::Tree::Filter::detail
{

static constexpr auto g_defaultRegexType = std::regex_constants::egrep | std::regex_constants::optimize;

bool matchesKey(Log const& log, Path const& path, std::regex const& re);
bool matchesValue(Log const& log, Path const& path, std::regex const& re);

std::vector<std::string> allValues(Log const& log, Path const& path);

}
