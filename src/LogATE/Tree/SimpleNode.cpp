#include "LogATE/Tree/SimpleNode.hpp"

namespace LogATE::Tree
{

void SimpleNode::insert(AnnotatedLog const& log)
{
  if( not matches(log) )
    return;
  logs().withLock()->insert(log.log_);
  insertToChildren(log);
}


SimpleNode::Children SimpleNode::children() const
{
  const Lock lock{mutex_};
  return children_;
}


bool SimpleNode::remove(NodeShPtr node)
{
  const Lock lock{mutex_};
  for(auto it=begin(children_); it!=end(children_); ++it)
    if( it->get() == node.get() )
    {
      children_.erase(it);
      return true;
    }
  return false;
}


NodeShPtr SimpleNode::addImpl(NodePtr node)
{
  auto shared = NodeShPtr{std::move(node)};
  {
    const Lock lock{mutex_};
    children_.push_back(shared);
  }
  passAllLogsToChild(shared);
  return shared;
}


namespace
{
void insertToChild(NodeShPtr const& child, AnnotatedLog const& log)
{
  try
  {
    child->insert(log);
  }
  catch(...)
  {
    // whatever...
  }
}
}


void SimpleNode::insertToChildren(AnnotatedLog const& log)
{
  for(auto c: children_)
    insertToChild(c, log);
}


namespace
{
auto copyAll(Logs const& logs)
{
  const auto& locked = logs.withLock();
  return std::vector<Log>{ locked->begin(), locked->end() };

}
}

void SimpleNode::passAllLogsToChild(NodeShPtr child)
{
  workers_->enqueue( [logs=copyAll(logs_), ptr=NodeWeakPtr{child.underlyingPointer()}] {
      for(auto const& log: logs)
      {
        auto sp = ptr.lock();   // weak pointer is crucial here, to avoid cyclic dependencies (node does keep thread pool!)
        if(not sp)
          return;
        insertToChild(NodeShPtr{sp}, AnnotatedLog{log});
      }
    } );
}

}
