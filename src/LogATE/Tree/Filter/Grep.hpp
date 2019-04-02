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

  // TODO: add inverted search flag
  Grep(Name name, Path path, std::string regex, Compare cmp, Case c = Case::Sensitive);

private:
  bool matches(Log const& log) const override;

  bool matchesAbsolute(Log const& log) const;
  bool matchesRelative(Log const& log) const;

  bool matchesAbsoluteKey(Log const& log) const;
  bool matchesAbsoluteValue(Log const& log) const;

  bool matchesRelativeKey(Log const& log) const;
  bool matchesRelativeValue(Log const& log) const;

  const Path path_;
  const Compare cmp_;
  const std::regex re_;
};

}
