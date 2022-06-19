#include "LogATE/Utils/matchesLog.hpp"
#include "LogATE/Utils/value2str.hpp"
#include "LogATE/Utils/forEachMatch.hpp"
#include "But/Optional.hpp"
#include <string>
#include <vector>

using LogATE::Tree::Path;


// TODO: there are a lot of searches, recursion and comparisons. there is a need for a fundamental change in an
//       underlying data structure, so that searches can be performed significantly faster with lesser (no?) allocations.
//       maybe using visitors for getting values could (potentially) help?

namespace LogATE::Utils
{

namespace
{
struct RegexKeySearch
{
  bool operator()(nlohmann::json const& json) const
  {
    if( json.is_object() )
      for(auto& e: json.items())
          if( std::regex_search(e.key(), *re_) )
            return false;
    return true;
  }

  std::regex const* re_{nullptr};
};
}


bool matchesKey(AnnotatedLog const& log, Path const& path, std::regex const& re)
{
  return not forEachMatch( log.json(), path, RegexKeySearch{&re} );
}



namespace
{
struct RegexValueSearch
{
  bool operator()(nlohmann::json const& json) const
  {
    const auto str = value2str(json);
    if(not str)
      return true;
    return not std::regex_search(*str, *re_);
  }

  std::regex const* re_{nullptr};
};
}


bool matchesValue(AnnotatedLog const& log, Path const& path, std::regex const& re)
{
  return not forEachMatch( log.json(), path, RegexValueSearch{&re} );
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


auto logMayContain(Log const& log, std::string const& str)
{
  return log.str().find(str) != std::string::npos;
}

auto logMayContain(Log const& log, std::string const& str1, std::string const& str2)
{
  return logMayContain(log, str1) && logMayContain(log, str2);
}
}


bool matchesAnyKey(Log const& log, std::string const& str)
{
  if( not logMayContain(log, str) )
    return false;
  return matchesAnyKeyRecursive(log.json(), StringSearch{&str});
}


bool matchesAnyKey(AnnotatedLog const& log, std::string const& str)
{
  if( not logMayContain(log.log(), str) )
    return false;
  return matchesAnyKeyRecursive(log.json(), StringSearch{&str});
}


bool matchesAnyValue(Log const& log, std::string const& str)
{
  if( not logMayContain(log, str) )
    return false;
  return matchesAnyValueRecursive(log.json(), StringSearch{&str}, LogATE::Utils::value2str);
}


bool matchesAnyValue(AnnotatedLog const& log, std::string const& str)
{
  if( not logMayContain(log.log(), str) )
    return false;
  return matchesAnyValueRecursive(log.json(), StringSearch{&str}, LogATE::Utils::value2str);
}


bool matchesAnyKeyValue(Log const& log, std::string const& key, std::string const& value)
{
  if( not logMayContain(log, key, value) )
    return false;
  return matchesAnyKeyValueRecursive(log.json(), StringSearch{&key}, StringSearch{&value}, LogATE::Utils::value2str);
}


bool matchesAnyKeyValue(AnnotatedLog const& log, std::string const& key, std::string const& value)
{
  if( not logMayContain(log.log(), key, value) )
    return false;
  return matchesAnyKeyValueRecursive(log.json(), StringSearch{&key}, StringSearch{&value}, LogATE::Utils::value2str);
}


namespace
{
struct GatherAllValues
{
  bool operator()(nlohmann::json const& node)
  {
    add(node);
    return true;
  }

  void add(nlohmann::json const& node)
  {
    if( node.is_null() )
      return;
    auto str = node2str(node);
    BUT_ASSERT( std::is_sorted( begin(out_), end(out_) ) );
    const auto it = std::lower_bound( begin(out_), end(out_), str );
    if( it != end(out_) && *it == str )
      return;
    out_.insert(it, std::move(str) );
    BUT_ASSERT( std::is_sorted( begin(out_), end(out_) ) );
  }

  static std::string node2str(nlohmann::json const& node)
  {
    BUT_ASSERT( not node.is_null() );
    auto tmp = ::LogATE::Utils::value2str(node);
    if(tmp)
      return *std::move(tmp);
    return node.dump();
  }

  std::vector<std::string> out_;
};

struct GatherAllLeafs
{
  bool operator()(nlohmann::json const& node)
  {
    if( node.is_null() || node.is_object() || node.is_array() )
      return true;
    return gav_(node);
  }

  GatherAllValues gav_;
};
}


std::vector<std::string> allValues(AnnotatedLog const& log, Path const& path)
{
  GatherAllLeafs gal;
  forEachMatch(log.json(), path, gal);
  return std::move(gal.gav_.out_);
}


std::vector<std::string> allNodeValues(AnnotatedLog const& log, Path const& path)
{
  GatherAllValues gav;
  forEachMatch(log.json(), path, gav);
  return std::move(gav.out_);
}

}
