#include "LogATE/Tree/Filter/Explode.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/Utils/matchesLog.hpp"

namespace LogATE::Tree::Filter
{

namespace
{
using Lock = std::lock_guard<std::mutex>;

auto acceptAllOutput(Utils::WorkerThreadsShPtr workers, std::string name, Node::TrimFields tf)
{
  return But::makeSharedNN<AcceptAll>( workers, Node::Name{std::move(name)}, std::move(tf) );
}
}

Explode::Explode(Utils::WorkerThreadsShPtr workers, Name name, Path path):
  Node{ std::move(workers), Type{"Explode"}, std::move(name), {} },
  path_{ std::move(path) },
  nonMatchingChild_{ acceptAllOutput( workers_, nonMatchingChildName().value_, trimFieldsExtended() ) }
{ }


bool Explode::insert(AnnotatedLog const& log)
{
  logs().withLock()->insert( log.log() );
  auto values = Utils::allNodeValues(log, path_);
  if( values.empty() )
  {
    nonMatchingChild_->insert(log);
    return true;
  }
  for(auto value: values)
    nodeFor( Name{value} )->insert(log);
  return true;
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


bool Explode::remove(NodeShPtr node)
{
  BUT_THROW(ExplicitNodeRemoveNotSupported, "while trying to remove node: " << node->name().value_);
}


NodeShPtr Explode::addImpl(NodePtr node)
{
  BUT_THROW(ExplicitNodeAddNotSupported, "while trying to add node: " << node->name().value_);
}


NodeShPtr Explode::nodeFor(Name const& name)
{
  const Lock lock{mutex_};
  auto it = children_.find(name);
  if( it == end(children_) )
    it = children_.insert( std::make_pair(name, acceptAllOutput( workers_, name.value_, trimFieldsExtended() )) ).first;
  return it->second;
}


Explode::TrimFields Explode::trimFieldsExtended() const
{
  auto tf = trimFields();
  tf.push_back(path_);
  return tf;
}

}
