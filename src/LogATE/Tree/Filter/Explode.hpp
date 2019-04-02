#pragma once
#include "LogATE/Tree/Node.hpp"
#include <vector>

namespace LogATE::Tree::Filter
{

class Explode: public Node
{
public:
  BUT_DEFINE_EXCEPTION(ExplicitNodeAddNotSupported, Node::Error, "explicit node addition not available for 'explode' node");

  Explode(Name name, Path path);

  void insert(Log const& log) override;
  Children children() const override;
  void add(NodePtr node) override;

private:
  const Path path_;
  std::vector<NodeShPtr> children_;
  const NodeShPtr nonMatchingChild_;
};

}
