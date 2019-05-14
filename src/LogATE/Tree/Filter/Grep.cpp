#include "LogATE/Tree/Filter/Grep.hpp"
#include "LogATE/Utils/matchesLog.hpp"
#include "But/Optional.hpp"

namespace LogATE::Tree::Filter
{

namespace
{
auto regexType(const Grep::Case c)
{
  const auto def = Utils::g_defaultRegexType;
  switch(c)
  {
    case Grep::Case::Sensitive:   return def;
    case Grep::Case::Insensitive: return def | std::regex_constants::icase;
  }
  throw std::logic_error{"unknown value of Grep::Case enum"};
}

Grep::TrimFields optTrim(const Grep::Trim trim, Path const& path)
{
  switch(trim)
  {
    case Grep::Trim::False: return {};
    case Grep::Trim::True: return {path};
  }
  throw std::logic_error{"unknown value of Grep::Trim enum"};
}
}

Grep::Grep(Utils::WorkerThreadsShPtr workers,
           Name name,
           Path path,
           std::string regex,
           const Compare cmp,
           const Case c,
           const Search search,
           const Trim trim):
  SimpleNode{ std::move(workers), Type{"Grep"}, std::move(name), optTrim(trim, path) },
  path_{ std::move(path) },
  cmp_{cmp},
  search_{search},
  re_{ std::move(regex), regexType(c) }
{ }


namespace
{
auto checkMatch(AnnotatedLog const& log, Path const& path, std::regex const& re, const Grep::Compare cmp)
{
  switch(cmp)
  {
    case Grep::Compare::Key:   return Utils::matchesKey(log, path, re);
    case Grep::Compare::Value: return Utils::matchesValue(log, path, re);
  }
  throw std::logic_error{"unknown value of Grep::Compare enum"};
}
}

bool Grep::matches(AnnotatedLog const& log) const
{
  const auto res = checkMatch(log, path_, re_, cmp_);
  switch(search_)
  {
    case Search::Regular: return res;
    case Search::Inverse: return not res;
  }
  throw std::logic_error{"unknown value of Grep::Search enum"};
}

}
