#pragma once
#include "LogATE/Log.hpp"
#include "LogATE/Tree/Path.hpp"
#include <vector>
#include <regex>

namespace LogATE::Utils
{

static constexpr auto g_defaultRegexType = std::regex_constants::egrep | std::regex_constants::optimize;

bool matchesKey(AnnotatedLog const& log, Tree::Path const& path, std::regex const& re);
bool matchesValue(AnnotatedLog const& log, Tree::Path const& path, std::regex const& re);

// TODO: add std::regex overloads of the below functions
bool matchesAnyKey(AnnotatedLog const& log, std::string const& str);
bool matchesAnyValue(AnnotatedLog const& log, std::string const& str);

/// works on key:value pairs only
std::vector<std::string> allValues(AnnotatedLog const& log, Tree::Path const& path);
/// works on nested nodes as well (serializes name as its value)
std::vector<std::string> allNodeValues(AnnotatedLog const& log, Tree::Path const& path);

}
