#include "LogATE/Tree/Filter/Explode.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/Tree/Filter/detail/matchesLog.hpp"

namespace LogATE::Tree::Filter
{

namespace
{
using Lock = std::lock_guard<std::mutex>;

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
  auto values = detail::allValues(log, path_);
  if( values.empty() )
  {
    const Lock lock{mutex_};
    nonMatchingChild_->insert(log);
    return;
  }
  for(auto value: values)
    nodeFor( Name{value} )->insert(log);
}

Explode::Children Explode::children() const
{
  Children out;
  out.reserve( children_.size() + 1 );
  out.push_back(nonMatchingChild_);
  const Lock lock{mutex_};
  for(auto child: children_)
    out.push_back(child.second);
  return out;
}

void Explode::add(NodePtr node)
{
  BUT_THROW(ExplicitNodeAddNotSupported, "while trying to add node: " << node->name().value_);
}

void Explode::remove(NodeShPtr node)
{
  BUT_THROW(ExplicitNodeRemoveNotSupported, "while trying to remove node: " << node->name().value_);
}

NodeShPtr Explode::nodeFor(Name const& name)
{
  const Lock lock{mutex_};
  auto it = children_.find(name);
  if( it == end(children_) )
    it = children_.insert( std::make_pair(name, acceptAllOutput(name.value_)) ).first;
  return it->second;
}

}
