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

/// works on key:value pairs only
std::vector<std::string> allValues(Log const& log, Path const& path);
/// works on nested nodes as well (serializes name as its value)
std::vector<std::string> allNodeValues(Log const& log, Path const& path);

}
