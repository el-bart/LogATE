#pragma once
#include "LogATE/Tree/Node.hpp"
#include <unordered_map>
#include <mutex>

namespace LogATE::Tree::Filter
{

class Explode: public Node
{
public:
  BUT_DEFINE_EXCEPTION(ExplicitNodeAddNotSupported, Node::Error, "explicit node addition not available for 'explode' node");
  BUT_DEFINE_EXCEPTION(ExplicitNodeRemoveNotSupported, Node::Error, "explicit node removal not available for 'explode' node");

  Explode(Utils::WorkerThreadsShPtr workers, Name name, Path path);

  bool insert(AnnotatedLog const& log) override;
  Children children() const override;
  bool remove(NodeShPtr node) override;

  static auto nonMatchingChildName() { return Name{"<unmatched>"}; }

private:
  struct NodeNameHash final
  {
    size_t operator()(Node::Name const& name) const { return hash_( name.value_ ); }
    std::hash<std::string> hash_;
  };

  NodeShPtr addImpl(NodePtr node) override;
  NodeShPtr nodeFor(Name const& name);
  TrimFields trimFieldsExtended() const;

  const Path path_;
  const NodeShPtr nonMatchingChild_;
  mutable std::mutex mutex_;
  std::unordered_map<Node::Name, NodeShPtr, NodeNameHash> children_;
};

}
