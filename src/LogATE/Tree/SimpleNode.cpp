#include "LogATE/Tree/SimpleNode.hpp"

namespace LogATE::Tree
{

SequenceNumber SimpleNode::insert(Log log)
{
  (void)log;
  throw 42;
}

SimpleNode::Children SimpleNode::children() const
{
  throw 42;
}

void SimpleNode::add(NodeShPtr node)
{
  (void)node;
  throw 42;
}

}
