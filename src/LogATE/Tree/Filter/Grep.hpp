#pragma once
#include "LogATE/Tree/SimpleNode.hpp"
#include <regex>

namespace LogATE::Tree::Filter
{

class Grep: public SimpleNode
{
public:
  enum class Compare
  {
    Key,
    Value
  };
  enum class Case
  {
    Sensitive,
    Insensitive
  };
  enum class Search
  {
    Regular,
    Inverse
  };

  Grep(Name name, Path path, std::string regex, Compare cmp, Case c = Case::Sensitive, Search search = Search::Regular);

private:
  bool matches(Log const& log) const override;

  const Path path_;
  const Compare cmp_;
  const Search search_;
  const std::regex re_;
};

}
