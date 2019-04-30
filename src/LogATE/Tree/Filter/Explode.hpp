#pragma once
#include "LogATE/Tree/Node.hpp"
#include <unordered_map>
#include <mutex>
#include <regex>

namespace LogATE::Tree::Filter
{

class Explode: public Node
{
public:
  BUT_DEFINE_EXCEPTION(ExplicitNodeAddNotSupported, Node::Error, "explicit node addition not available for 'explode' node");
  BUT_DEFINE_EXCEPTION(ExplicitNodeRemoveNotSupported, Node::Error, "explicit node removal not available for 'explode' node");

  Explode(Utils::WorkerThreadsShPtr workers, Name name, Path path);

  void insert(Log const& log) override;
  Children children() const override;
  NodeShPtr add(NodePtr node) override;
  bool remove(NodeShPtr node) override;

  static auto nonMatchingChildName() { return Name{"<unmatched>"}; }

private:
  struct NodeNameHash final
  {
    size_t operator()(Node::Name const& name) const { return hash_( name.value_ ); }
    std::hash<std::string> hash_;
  };

  NodeShPtr nodeFor(Name const& name);

  const Path path_;
  const NodeShPtr nonMatchingChild_;
  const std::regex matchAny_;
  mutable std::mutex mutex_;
  std::unordered_map<Node::Name, NodeShPtr, NodeNameHash> children_;
};

}
