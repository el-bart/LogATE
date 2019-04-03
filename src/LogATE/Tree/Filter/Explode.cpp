#include "LogATE/Tree/Filter/Explode.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/Tree/Filter/detail/matchesLog.hpp"

namespace LogATE::Tree::Filter
{

namespace
{
auto acceptAllOutput(std::string name)
{
  return But::makeSharedNN<AcceptAll>( Node::Name{std::move(name)} );
}
}

Explode::Explode(Name name, Path path):
  Node{ Type{"Explode"}, std::move(name), {path}},
  path_{std::move(path)},
  nonMatchingChild_{ acceptAllOutput( nonMatchingChildName().value_ ) },
  matchAny_{"", detail::g_defaultRegexType}
{ }

void Explode::insert(Log const& log)
{
  (void)log;
  throw 41;
}

Explode::Children Explode::children() const
{
  Children out;
  out.push_back(nonMatchingChild_);
  return out;
}

void Explode::add(NodePtr node)
{
  BUT_THROW(ExplicitNodeAddNotSupported, "while trying to add node: " << node->name().value_);
}

}
