#include "LogATE/Tree/Filter/Grep.hpp"
#include "LogATE/Tree/Filter/detail/matchesLog.hpp"
#include "But/Optional.hpp"

// TODO: arrays are ignored for now (i.e. nothing is searched inside them) - this should change
// TODO: there are a lot of searches, recursion and comparisons. there is a need for a fundamental change in an
//       underlying data structure, so that searches can be performed significantly faster with lesser (no?) allocations.

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

Grep::Grep(Name name, Path path, std::string regex, const Compare cmp, const Case c):
  SimpleNode{ Type{"grep"}, std::move(name), TrimFields{path} },
  path_{ std::move(path) },
  cmp_{cmp},
  re_{ std::move(regex), regexType(c) }
{ }


bool Grep::matches(Log const& log) const
{
  switch(cmp_)
  {
    case Compare::Key:   return detail::matchesKey(log, path_, re_);
    case Compare::Value: return detail::matchesValue(log, path_, re_);
  }
}

}
