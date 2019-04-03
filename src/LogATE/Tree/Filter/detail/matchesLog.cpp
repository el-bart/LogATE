#include "LogATE/Tree/Filter/detail/matchesLog.hpp"
#include "But/Optional.hpp"
#include <string>
#include <vector>

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

template<typename F>
bool hasMatchingKey(nlohmann::json const& node, F const& cmp)
{
  if( not node.is_object() )
    return false;
  for(auto it=node.begin(); it!=node.end(); ++it)
    if( cmp( it.key() ) )
      return true;
  return false;
}

template<typename F>
auto matchesAbsoluteKey(Log const& log, Path const& path, F const& cmp)
{
  const auto n = getNodeByPath(log, path.begin()+1, path.end());
  return hasMatchingKey(n, cmp);
}

template<typename F>
auto matchesAbsoluteValue(Log const& log, Path const& path, F const& cmp)
{
  const auto n = getNodeByPath(log, path.begin()+1, path.end());
  const auto str = value2str(n);
  if(not str)
    return false;
  return cmp(*str);
}


template<typename F>
bool matchesRelativeKeyDirect(nlohmann::json const& log, Path const& path, F const& cmp)
{
  const auto n = getNodeByPath(log, path.begin(), path.end());
  return hasMatchingKey(n, cmp);
}

template<typename F>
bool matchesRelativeKeyRecursive(nlohmann::json const& log, Path const& path, F const& cmp);

template<typename F>
bool matchesRelativeKeyInDirectChildren(nlohmann::json const& log, Path const& path, F const& cmp)
{
  for(auto it=log.begin(); it!=log.end(); ++it)
    if( matchesRelativeKeyRecursive(*it, path, cmp) )
      return true;
  return false;
}

template<typename F>
bool matchesRelativeKeyRecursive(nlohmann::json const& log, Path const& path, F const& cmp)
{
  if( matchesRelativeKeyDirect(log, path, cmp) )
    return true;
  if( log.is_boolean() || log.is_number() || log.is_string() )
    return false;
  if( matchesRelativeKeyInDirectChildren(log, path, cmp) )
    return true;
  return false;
}

template<typename F>
auto matchesRelativeKey(Log const& log, Path const& path, F const& cmp)
{
  return matchesRelativeKeyRecursive(*log.log_, path, cmp);
}


template<typename F>
bool matchesRelativeValueRecursive(nlohmann::json const& log, Path const& path, F const& cmp)
{
  if( not log.is_object() && not log.is_array() )
    return false;

  {
    const auto n = getNodeByPath(log, path.begin(), path.end());
    const auto str = value2str(n);
    if(str && cmp(*str))
      return true;
  }

  for(auto it=log.begin(); it!=log.end(); ++it)
    if( matchesRelativeValueRecursive(*it, path, cmp) )
      return true;
  return false;
}

template<typename F>
auto matchesRelativeValue(Log const& log, Path const& path, F const& cmp)
{
  return matchesRelativeValueRecursive(*log.log_, path, cmp);
}


template<typename F>
bool matchesKeyImpl(Log const& log, Path const& path, F const& cmp)
{
  if( path.value_.empty() )
    return false;
  if( path.root() )
    return matchesAbsoluteKey(log, path, cmp);
  return matchesRelativeKey(log, path, cmp);
}

template<typename F>
bool matchesValueImpl(Log const& log, Path const& path, F const& cmp)
{
  if( path.value_.empty() )
    return false;
  if( path.root() )
    return matchesAbsoluteValue(log, path, cmp);
  return matchesRelativeValue(log, path, cmp);
}

struct RegexCompare
{
  bool operator()(std::string const& str) const { return std::regex_search(str, *re_); }
  std::regex const* re_{nullptr};
};

}


bool matchesKey(Log const& log, Path const& path, std::regex const& re)
{
  return matchesKeyImpl(log, path, RegexCompare{&re});
}


bool matchesValue(Log const& log, Path const& path, std::regex const& re)
{
  return matchesValueImpl(log, path, RegexCompare{&re});
}


namespace
{
struct GatherAllValues
{
  bool operator()(std::string const& str) const
  {
    BUT_ASSERT(out_ != nullptr);
    BUT_ASSERT( std::is_sorted( begin(*out_), end(*out_) ) );
    const auto it = std::lower_bound( begin(*out_), end(*out_), str );
    if( it != end(*out_) && *it == str )
      return false;
    out_->insert(it, str);
    BUT_ASSERT( std::is_sorted( begin(*out_), end(*out_) ) );
    return false;
  }

  std::vector<std::string>* out_{nullptr};
};
}

std::vector<std::string> allValues(Log const& log, Path const& path)
{
  std::vector<std::string> out;
  matchesValueImpl(log, path, GatherAllValues{&out});
  return out;
}

}
