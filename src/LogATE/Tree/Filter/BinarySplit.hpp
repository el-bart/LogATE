#pragma once
#include "LogATE/Tree/Node.hpp"

namespace LogATE::Tree::Filter
{

class BinarySplit: public Node
{
public:
  BUT_DEFINE_EXCEPTION(ExplicitNodeAddNotSupported, Node::Error, "explicit node addition not available for 'binary split' node");
  BUT_DEFINE_EXCEPTION(ExplicitNodeRemoveNotSupported, Node::Error, "explicit node removal not available for 'binary split' node");

  BinarySplit(Utils::WorkerThreadsShPtr workers, Name name, NodeShPtr matched);

  bool insert(AnnotatedLog const& log) override;
  Children children() const override;
  bool remove(NodeShPtr node) override;

  static auto nonMatchingChildName() { return Name{"<unmatched>"}; }

private:
  NodeShPtr addImpl(NodePtr node) override;

  const NodeShPtr matched_;
  const NodeShPtr unmatched_;
};

}
