#include "LogATE/Tree/Filter/Explode.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/Tree/Filter/detail/matchesLog.hpp"

namespace LogATE::Tree::Filter
{

namespace
{
using Lock = std::lock_guard<std::mutex>;

auto acceptAllOutput(Utils::WorkerThreadsShPtr workers, std::string name)
{
  return But::makeSharedNN<AcceptAll>( workers, Node::Name{std::move(name)} );
}
}

Explode::Explode(Utils::WorkerThreadsShPtr workers, Name name, Path path):
  Node{ std::move(workers), Type{"Explode"}, std::move(name), {path}},
  path_{std::move(path)},
  nonMatchingChild_{ acceptAllOutput( workers_, nonMatchingChildName().value_ ) },
  matchAny_{"", detail::g_defaultRegexType}
{ }

void Explode::insert(Log const& log)
{
  logs().withLock()->insert(log);
  auto values = detail::allNodeValues(log, path_);
  if( values.empty() )
  {
    const Lock lock{mutex_};
    nonMatchingChild_->insert(log);
    return;
  }
  for(auto value: values)
    nodeFor( Name{value} )->insert(log);
}


namespace
{
template<typename C>
auto sorted(C const& in, std::mutex& mutex)
{
  std::map<Node::Name, NodeShPtr> out;
  const Lock lock{mutex};
  for(auto& e: in)
    out.insert(e);
  return out;
}
}

Explode::Children Explode::children() const
{
  Children out;
  out.reserve( children_.size() + 1 );
  out.push_back(nonMatchingChild_);
  for(auto& child: sorted(children_, mutex_))
    out.push_back(child.second);
  return out;
}

NodeShPtr Explode::add(NodePtr node)
{
  BUT_THROW(ExplicitNodeAddNotSupported, "while trying to add node: " << node->name().value_);
}

bool Explode::remove(NodeShPtr node)
{
  BUT_THROW(ExplicitNodeRemoveNotSupported, "while trying to remove node: " << node->name().value_);
}

NodeShPtr Explode::nodeFor(Name const& name)
{
  const Lock lock{mutex_};
  auto it = children_.find(name);
  if( it == end(children_) )
    it = children_.insert( std::make_pair(name, acceptAllOutput(workers_, name.value_)) ).first;
  return it->second;
}

}
