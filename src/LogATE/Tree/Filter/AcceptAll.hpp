#pragma once
#include "LogATE/Tree/SimpleNode.hpp"
#include <regex>

namespace LogATE::Tree::Filter
{

class AcceptAll: public SimpleNode
{
public:
  explicit AcceptAll(Name name);

private:
  bool matches(Log const& log) const override;
};

}
