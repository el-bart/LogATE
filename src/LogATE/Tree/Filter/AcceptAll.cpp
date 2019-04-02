#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include <regex>

namespace LogATE::Tree::Filter
{

AcceptAll::AcceptAll(Name name):
  SimpleNode{Type{"AcceptAll"}, std::move(name), {}}
{
}

bool AcceptAll::matches(Log const&) const
{
  return true;
}

}
