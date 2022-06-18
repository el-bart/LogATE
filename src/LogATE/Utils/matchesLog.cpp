#include "LogATE/Utils/matchesLog.hpp"
#include "LogATE/Utils/value2str.hpp"
#include "But/Optional.hpp"
#include <string>
#include <vector>

using LogATE::Tree::Path;


// TODO[array]: arrays are ignored for now (i.e. nothing is searched inside them) - this should change
// TODO: there are a lot of searches, recursion and comparisons. there is a need for a fundamental change in an
//       underlying data structure, so that searches can be performed significantly faster with lesser (no?) allocations.

namespace LogATE::Utils
{

namespace
{

using PathIter = Path::Data::const_iterator;

nlohmann::json getNodeByPath(nlohmann::json const& n, PathIter pathBegin, PathIter pathEnd)
{
  // TODO[array]: add array processing here
  auto ptr = &n;
  for(auto it=pathBegin; it!=pathEnd; ++it)
  {
    const auto p = ptr->find( it->name() ); // TODO[array]: will not work for wildcards, or elements of arrays...
    if( p == ptr->end() )
      return {};
    ptr = &*p;
  }
  return *ptr;
}

nlohmann::json getNodeByPath(AnnotatedLog const& log, PathIter pathBegin, PathIter pathEnd)
{
  return getNodeByPath(log.json(), pathBegin, pathEnd);
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
auto matchesAbsoluteKey(AnnotatedLog const& log, Path const& path, F const& cmp)
{
  const auto n = getNodeByPath(log, path.begin(), path.end());
  return hasMatchingKey(n, cmp);
}

template<typename F, typename ToStr>
auto matchesAbsoluteValue(AnnotatedLog const& log, Path const& path, F const& cmp, ToStr const& toStr)
{
  const auto n = getNodeByPath(log, path.begin(), path.end());
  const auto str = toStr(n);
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
auto matchesRelativeKey(AnnotatedLog const& log, Path const& path, F const& cmp)
{
  return matchesRelativeKeyRecursive(log.json(), path, cmp);
}


template<typename F, typename ToStr>
bool matchesRelativeValueRecursive(nlohmann::json const& log, Path const& path, F const& cmp, ToStr const& toStr)
{
  if( not log.is_object() && not log.is_array() )
    return false;

  {
    const auto n = getNodeByPath(log, path.begin(), path.end());
    const auto str = toStr(n);
    if(str && cmp(*str))
      return true;
  }

  for(auto it=log.begin(); it!=log.end(); ++it)
    if( matchesRelativeValueRecursive(*it, path, cmp, toStr) )
      return true;
  return false;
}

template<typename F, typename ToStr>
auto matchesRelativeValue(AnnotatedLog const& log, Path const& path, F const& cmp, ToStr const& toStr)
{
  return matchesRelativeValueRecursive(log.json(), path, cmp, toStr);
}


template<typename F>
bool matchesKeyImpl(AnnotatedLog const& log, Path const& path, F const& cmp)
{
  if( path.empty() )
    return false;
  if( path.absolute() )
    return matchesAbsoluteKey(log, path, cmp);
  return matchesRelativeKey(log, path, cmp);
}

template<typename F, typename ToStr>
bool matchesValueImpl(AnnotatedLog const& log, Path const& path, F const& cmp, ToStr const& toStr)
{
  if( path.empty() )
    return false;
  if( path.absolute() )
    return matchesAbsoluteValue(log, path, cmp, toStr);
  return matchesRelativeValue(log, path, cmp, toStr);
}

struct RegexCompare
{
  bool operator()(std::string const& str) const { return std::regex_search(str, *re_); }
  std::regex const* re_{nullptr};
};

}


bool matchesKey(AnnotatedLog const& log, Path const& path, std::regex const& re)
{
  return matchesKeyImpl(log, path, RegexCompare{&re});
}


bool matchesValue(AnnotatedLog const& log, Path const& path, std::regex const& re)
{
  return matchesValueImpl(log, path, RegexCompare{&re}, LogATE::Utils::value2str);
}


namespace
{
template<typename F>
bool matchesAnyKeyRecursive(nlohmann::json const& log, F const& cmp)
{
  if( log.is_null() )
    return false;

  if( log.is_array() )
    for(auto it=log.begin(); it!=log.end(); ++it)
      if( matchesAnyKeyRecursive(*it, cmp) )
        return true;

  if( log.is_object() )
    for(auto it=log.begin(); it!=log.end(); ++it)
    {
      if( cmp( it.key() ) )
        return true;
      if( matchesAnyKeyRecursive(*it, cmp) )
        return true;
    }

  return false;
}

template<typename F, typename ToStr>
bool matchesAnyValueRecursive(nlohmann::json const& log, F const& cmp, ToStr const& toStr)
{
  if( log.is_null() )
    return false;

  if( log.is_object() || log.is_array() )
  {
    for(auto it=log.begin(); it!=log.end(); ++it)
      if( matchesAnyValueRecursive(*it, cmp, toStr) )
        return true;
  }
  else
  {
    const auto str = toStr(log);
    if(str && cmp(*str))
      return true;
  }
  return false;
}

template<typename FK, typename FV, typename ToStr>
bool matchesAnyKeyValueRecursive(nlohmann::json const& log, FK const& cmpKey, FV const& cmpValue, ToStr const& toStr)
{
  if( log.is_null() )
    return false;

  if( log.is_array() )
    for(auto it=log.begin(); it!=log.end(); ++it)
      if( matchesAnyKeyValueRecursive(*it, cmpKey, cmpValue, toStr) )
        return true;

  if( log.is_object() )
    for(auto it=log.begin(); it!=log.end(); ++it)
    {
      if( cmpKey( it.key() ) )
      {
        const auto v = toStr( it.value() );
        if( v && cmpValue(*v) )
          return true;
      }
      if( matchesAnyKeyValueRecursive(*it, cmpKey, cmpValue, toStr) )
        return true;
    }

  return false;
}

struct StringSearch
{
  bool operator()(std::string const& str) const { return str.find(*query_) != std::string::npos; }
  std::string const* query_{nullptr};
};
}


bool matchesAnyKey(Log const& log, std::string const& str)
{
  if( log.str().find(str) == std::string::npos )
    return false;
  return matchesAnyKey( AnnotatedLog{log}, str );
}


bool matchesAnyKey(AnnotatedLog const& log, std::string const& str)
{
  return matchesAnyKeyRecursive(log.json(), StringSearch{&str});
}


bool matchesAnyValue(Log const& log, std::string const& str)
{
  if( log.str().find(str) == std::string::npos )
    return false;
  return matchesAnyValue( AnnotatedLog{log}, str );
}


bool matchesAnyValue(AnnotatedLog const& log, std::string const& str)
{
  return matchesAnyValueRecursive(log.json(), StringSearch{&str}, LogATE::Utils::value2str);
}


bool matchesAnyKeyValue(Log const& log, std::string const& key, std::string const& value)
{
  if( log.str().find(key) == std::string::npos )
    return false;
  if( log.str().find(value) == std::string::npos )
    return false;
  return matchesAnyKeyValue( AnnotatedLog{log}, key, value );
}


bool matchesAnyKeyValue(AnnotatedLog const& log, std::string const& key, std::string const& value)
{
  return matchesAnyKeyValueRecursive(log.json(), StringSearch{&key}, StringSearch{&value}, LogATE::Utils::value2str);
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

std::vector<std::string> allValues(AnnotatedLog const& log, Path const& path)
{
  std::vector<std::string> out;
  matchesValueImpl(log, path, GatherAllValues{&out}, LogATE::Utils::value2str);
  return out;
}

namespace
{
But::Optional<std::string> dumpStruct(nlohmann::json const& node)
{
  if( node.is_null() )
    return {};
  auto tmp = LogATE::Utils::value2str(node);
  if(tmp)
    return tmp;
  return node.dump();
}
}

std::vector<std::string> allNodeValues(AnnotatedLog const& log, Path const& path)
{
  std::vector<std::string> out;
  matchesValueImpl(log, path, GatherAllValues{&out}, dumpStruct);
  return out;
}

}
