#include "LogATE/Tree/Filter/Grep.hpp"
#include "But/Optional.hpp"

namespace LogATE::Tree::Filter
{

namespace
{
auto regexType(const Grep::Case c)
{
  const auto def = std::regex_constants::optimize | std::regex_constants::egrep;
  switch(c)
  {
    case Grep::Case::Sensitive:   return def;
    case Grep::Case::Insensitive: return def | std::regex_constants::icase;
  }
}
}

Grep::Grep(Name name, Path path, std::string regex, const Compare cmp, const Case c):
  SimpleNode{ Type{"grep"}, std::move(name), TrimFields{path} },
  path_{ std::move(path) },
  cmp_{cmp},
  re_{ std::move(regex), regexType(c) }
{ }


bool Grep::matches(Log const& log) const
{
  if( path_.value_.empty() )
    return false;
  if( path_.root() )
    return matchesAbsolute(log);
  return matchesRelative(log);
}

bool Grep::matchesAbsolute(Log const& log) const
{
  switch(cmp_)
  {
    case Compare::Key:   return matchesAbsoluteKey(log);
    case Compare::Value: return matchesAbsoluteValue(log);
  }
}

bool Grep::matchesRelative(Log const& log) const
{
  switch(cmp_)
  {
    case Compare::Key:   return matchesRelativeKey(log);
    case Compare::Value: return matchesRelativeValue(log);
  }
}

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
  if( node.is_null() )
    return false;
  for(auto it=node.begin(); it!=node.end(); ++it)
    if( std::regex_search(it.key(), re) )
      return true;
  return false;
}
}

bool Grep::matchesAbsoluteKey(Log const& log) const
{
  const auto n = getNodeByPath(log, path_.begin()+1, path_.end());
  if( n.is_null() )
    return false;
  for(auto it=n.begin(); it!=n.end(); ++it)
    if( std::regex_search(it.key(), re_) )
      return true;
  return false;
}

bool Grep::matchesAbsoluteValue(Log const& log) const
{
  const auto n = getNodeByPath(log, path_.begin()+1, path_.end());
  const auto str = value2str(n);
  if(not str)
    return false;
  return std::regex_search(*str, re_);
}


namespace
{
bool matchesRelativeKeyRecursive(nlohmann::json const& log, Path const& path, std::regex const& re)
{
  if( log.is_null() )
    return false;

  {
    const auto n = getNodeByPath(log, path.begin(), path.end());
    const auto str = value2str(n);
    if(str && std::regex_search(*str, re))
      return true;
  }

  for(auto it=log.begin(); it!=log.end(); ++it)
    if( matchesRelativeKeyRecursive(*it, path, re) )
      return true;
  return false;
}
}

bool Grep::matchesRelativeKey(Log const& log) const
{
  throw 42;                                                                                         
  return matchesRelativeKeyRecursive(*log.log_, path_, re_);
}


namespace
{
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
}

bool Grep::matchesRelativeValue(Log const& log) const
{
  return matchesRelativeValueRecursive(*log.log_, path_, re_);
}

}
