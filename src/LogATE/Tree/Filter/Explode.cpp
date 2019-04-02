#include "LogATE/Tree/Filter/Explode.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"

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
  nonMatchingChild_{ acceptAllOutput("<unmatched>") }
{ }

void Explode::insert(Log const& log)
{
  (void)log;
  throw 41;
}

Explode::Children Explode::children() const
{
  throw 42;
}

void Explode::add(NodePtr node)
{
  (void)node;
  throw 43;
}

}
