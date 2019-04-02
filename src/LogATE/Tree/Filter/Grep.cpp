#include "LogATE/Tree/Filter/Grep.hpp"
#include "LogATE/Tree/Filter/detail/matchesLog.hpp"
#include "But/Optional.hpp"

namespace LogATE::Tree::Filter
{

namespace
{
auto regexType(const Grep::Case c)
{
  const auto def = detail::g_defaultRegexType;
  switch(c)
  {
    case Grep::Case::Sensitive:   return def;
    case Grep::Case::Insensitive: return def | std::regex_constants::icase;
  }
}
}

Grep::Grep(Name name, Path path, std::string regex, const Compare cmp, const Case c, const Search search):
  SimpleNode{ Type{"grep"}, std::move(name), TrimFields{path} },
  path_{ std::move(path) },
  cmp_{cmp},
  search_{search},
  re_{ std::move(regex), regexType(c) }
{ }


namespace
{
auto checkMatch(Log const& log, Path const& path, std::regex const& re, const Grep::Compare cmp)
{
  switch(cmp)
  {
    case Grep::Compare::Key:   return detail::matchesKey(log, path, re);
    case Grep::Compare::Value: return detail::matchesValue(log, path, re);
  }
}
}

bool Grep::matches(Log const& log) const
{
  const auto res = checkMatch(log, path_, re_, cmp_);
  switch(search_)
  {
    case Search::Regular: return res;
    case Search::Inverse: return not res;
  }
}

}
