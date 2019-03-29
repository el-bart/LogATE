#include "LogATE/Tree/SimpleNode.hpp"

namespace LogATE::Tree
{

void SimpleNode::insert(Log const& log)
{
  if( not matches(log) )
    return;
  logs().withLock()->insert(log);
}

SimpleNode::Children SimpleNode::children() const
{
  const Lock lock{mutex_};
  return children_;
}

void SimpleNode::add(NodePtr node)
{
  const Lock lock{mutex_};
  children_.push_back( std::move(node) );
}

}
