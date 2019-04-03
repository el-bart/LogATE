#pragma once
#include "LogATE/Tree/Node.hpp"
#include <unordered_set>
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
  struct NodeHash final
  {
    size_t operator()(NodeShPtr const& node) const { return hash_( node->name().value_ ); }
    std::hash<std::string> hash_;
  };

  struct NodeCompare final
  {
    bool operator()(NodeShPtr const& lhs, NodeShPtr const& rhs) const { return lhs.get() == rhs.get(); }
  };

  const Path path_;
  std::unordered_set<NodeShPtr, NodeHash, NodeCompare> children_;
  const NodeShPtr nonMatchingChild_;
  const std::regex matchAny_;
};

}
