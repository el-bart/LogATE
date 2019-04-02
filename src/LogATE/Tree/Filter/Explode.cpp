#include "LogATE/Tree/Filter/Explode.hpp"

namespace LogATE::Tree::Filter
{

Explode::Explode(Name name, Path path):
  Node{ Type{"Explode"}, std::move(name), {path}},
  path_{std::move(path)}
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
