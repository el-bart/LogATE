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
