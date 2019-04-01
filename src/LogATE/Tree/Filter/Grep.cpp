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

But::Optional<std::string> key2str(nlohmann::json const& node)
{
  (void)node;
  throw 42;
  return {};
}

template<typename F>
bool matchesAbsoluteComparator(Log const& log, Path const& path, std::regex const& re, F const& extract)
{
  const auto n = getNodeByPath(log, path.begin()+1, path.end());
  const auto str = extract(n);
  if(not str)
    return false;
  return std::regex_search(*str, re);
}
}

bool Grep::matchesAbsoluteKey(Log const& log) const
{
  (void)log;
  throw std::runtime_error{"N/A :P"};
}

bool Grep::matchesAbsoluteValue(Log const& log) const
{
  return matchesAbsoluteComparator(log, path_, re_, value2str);
}

namespace
{
template<typename F>
bool matchesRelativeValueRecursive(nlohmann::json const& log, Path const& path, std::regex const& re, F const& extract)
{
  if( not log.is_object() && not log.is_array() )
    return false;

  {
    const auto n = getNodeByPath(log, path.begin(), path.end());
    const auto str = extract(n);
    if(str && std::regex_search(*str, re))
      return true;
  }

  for(auto it=log.begin(); it!=log.end(); ++it)
    if( matchesRelativeValueRecursive(*it, path, re, extract) )
      return true;
  return false;
}
}

bool Grep::matchesRelativeKey(Log const& log) const
{
  key2str({});      // TODO...  
  (void)log;
  throw std::runtime_error{"N/A :P"};
}

bool Grep::matchesRelativeValue(Log const& log) const
{
  return matchesRelativeValueRecursive(*log.log_, path_, re_, value2str);
}

}
