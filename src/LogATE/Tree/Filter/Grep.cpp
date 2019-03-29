#include "LogATE/Tree/Filter/Grep.hpp"

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

Grep::Grep(Name name, Path path, std::string regex, Compare cmp, Case c):
  SimpleNode{ Type{"grep"}, std::move(name), TrimFields{path} },
  path_{ std::move(path) },
  cmp_{cmp},
  re_{ std::move(regex), regexType(c) }
{ }


bool Grep::matches(Log const& log) const
{
  (void)cmp_;
  (void)log;
  throw std::runtime_error{"not implemented yet :P"};
}

}
