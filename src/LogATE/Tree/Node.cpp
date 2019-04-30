#include "LogATE/Tree/Node.hpp"

namespace LogATE::Tree
{

void Node::pruneUpTo(const SequenceNumber sn)
{
  logs().withLock()->pruneUpTo(sn);
  for(auto child: children())
    workers_->enqueue( [child,sn] { child->pruneUpTo(sn); } );
}

}
