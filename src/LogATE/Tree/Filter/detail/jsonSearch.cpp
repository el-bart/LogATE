#include "LogATE/Tree/Filter/Grep.hpp"
#include "But/Optional.hpp"

// TODO: arrays are ignored for now (i.e. nothing is searched inside them) - this should change
// TODO: there are a lot of searches, recursion and comparisons. there is a need for a fundamental change in an
//       underlying data structure, so that searches can be performed significantly faster with lesser (no?) allocations.

namespace LogATE::Tree::Filter::detail
{

namespace
{

using PathIter = std::vector<std::string>::const_iterator;

nlohmann::json getNodeByPath(nlohmann::json n, PathIter pathBegin, PathIter pathEnd)
{
  for(auto it=pathBegin; it!=pathEnd; ++it)
  {
    const auto p = n.find(*it);
    if( p == n.end() )
      return {};
    n = *p;
  }
  return n;
}

nlohmann::json getNodeByPath(Log const& log, PathIter pathBegin, PathIter pathEnd)
{
  return getNodeByPath(*log.log_, pathBegin, pathEnd);
}

But::Optional<std::string> value2str(nlohmann::json const& node)
{
  if( node.is_string() )
    return node.get<std::string>();
  if( node.is_number() )
    return std::to_string( node.get<double>() );
  if( node.is_boolean() )
    return std::string{ node.get<bool>() ? "true" : "false" };
  return {};
}

bool hasMatchingKey(nlohmann::json const& node, std::regex const& re)
{
  if( not node.is_object() )
    return false;
  for(auto it=node.begin(); it!=node.end(); ++it)
    if( std::regex_search(it.key(), re) )
      return true;
  return false;
}

auto matchesAbsoluteKey(Log const& log, Path const& path, std::regex const& re)
{
  const auto n = getNodeByPath(log, path.begin()+1, path.end());
  return hasMatchingKey(n, re);
}

auto matchesAbsoluteValue(Log const& log, Path const& path, std::regex const& re)
{
  const auto n = getNodeByPath(log, path.begin()+1, path.end());
  const auto str = value2str(n);
  if(not str)
    return false;
  return std::regex_search(*str, re);
}


bool matchesRelativeKeyDirect(nlohmann::json const& log, Path const& path, std::regex const& re)
{
  const auto n = getNodeByPath(log, path.begin(), path.end());
  return hasMatchingKey(n, re);
}

bool matchesRelativeKeyRecursive(nlohmann::json const& log, Path const& path, std::regex const& re);

bool matchesRelativeKeyInDirectChildren(nlohmann::json const& log, Path const& path, std::regex const& re)
{
  for(auto it=log.begin(); it!=log.end(); ++it)
    if( matchesRelativeKeyRecursive(*it, path, re) )
      return true;
  return false;
}

bool matchesRelativeKeyRecursive(nlohmann::json const& log, Path const& path, std::regex const& re)
{
  if( matchesRelativeKeyDirect(log, path, re) )
    return true;
  if( log.is_boolean() || log.is_number() || log.is_string() )
    return false;
  if( matchesRelativeKeyInDirectChildren(log, path, re) )
    return true;
  return false;
}

auto matchesRelativeKey(Log const& log, Path const& path, std::regex const& re)
{
  return matchesRelativeKeyRecursive(*log.log_, path, re);
}


bool matchesRelativeValueRecursive(nlohmann::json const& log, Path const& path, std::regex const& re)
{
  if( not log.is_object() && not log.is_array() )
    return false;

  {
    const auto n = getNodeByPath(log, path.begin(), path.end());
    const auto str = value2str(n);
    if(str && std::regex_search(*str, re))
      return true;
  }

  for(auto it=log.begin(); it!=log.end(); ++it)
    if( matchesRelativeValueRecursive(*it, path, re) )
      return true;
  return false;
}

auto matchesRelativeValue(Log const& log, Path const& path, std::regex const& re)
{
  return matchesRelativeValueRecursive(*log.log_, path, re);
}

}


bool matchesKey(Log const& log, Path const& path, std::regex const& re)
{
  if( path.value_.empty() )
    return false;
  if( path.root() )
    return matchesAbsoluteKey(log, path, re);
  return matchesRelativeKey(log, path, re);
}


bool matchesValue(Log const& log, Path const& path, std::regex const& re)
{
  if( path.value_.empty() )
    return false;
  if( path.root() )
    return matchesAbsoluteValue(log, path, re);
  return matchesRelativeValue(log, path, re);
}

}
