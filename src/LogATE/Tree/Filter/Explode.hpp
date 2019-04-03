#pragma once
#include "LogATE/Tree/Node.hpp"
#include <unordered_map>
#include <regex>

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

  static auto nonMatchingChildName() { return Name{"<unmatched>"}; }

private:
  struct NodeNameHash final
  {
    size_t operator()(Node::Name const& name) const { return hash_( name.value_ ); }
    std::hash<std::string> hash_;
  };

  const Path path_;
  std::unordered_map<Node::Name, NodeShPtr, NodeNameHash> children_;
  const NodeShPtr nonMatchingChild_;
  const std::regex matchAny_;
};

}
