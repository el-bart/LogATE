#include "LogATE/Tree/Node.hpp"

namespace LogATE::Tree
{

NodeShPtr Node::add(NodePtr node)
{
  BUT_ASSERT(node);
  node->trimAdditionalFields( trimFields() );
  return addImpl( std::move(node) );
}

Node::TrimFields Node::trimFields() const
{
  const std::lock_guard<std::mutex> lock{trimFieldsMutex_};
  return trimFields_;
}


void Node::pruneUpTo(Log::Key const& key)
{
  logs().withLock()->pruneUpTo(key);
  for(auto child: children())
    workers_->enqueue( [child,key] { child->pruneUpTo(key); } );
}


void Node::trimAdditionalFields(TrimFields const& other)
{
  {
    const std::lock_guard<std::mutex> lock{trimFieldsMutex_};
    trimFields_.insert( end(trimFields_), begin(other), end(other) );
  }
  for(auto& c: children())
    c->trimAdditionalFields(other);
}

}
