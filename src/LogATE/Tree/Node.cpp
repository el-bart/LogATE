#include "LogATE/Tree/Node.hpp"

namespace LogATE::Tree
{

Node::TrimFields Node::trimFields() const
{
  const std::lock_guard<std::mutex> lock{trimFieldsMutex_};
  return trimFields_;
}


void Node::trimAdditionalFields(TrimFields const& other)
{
  const std::lock_guard<std::mutex> lock{trimFieldsMutex_};
  trimFields_.insert( end(trimFields_), begin(other), end(other) );
}


void Node::pruneUpTo(const SequenceNumber sn)
{
  logs().withLock()->pruneUpTo(sn);
  for(auto child: children())
    workers_->enqueue( [child,sn] { child->pruneUpTo(sn); } );
}

}
