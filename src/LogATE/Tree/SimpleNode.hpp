#pragma once
#include "LogATE/Tree/Node.hpp"

namespace LogATE::Tree
{

class SimpleNode: public Node
{
public:
  bool insert(AnnotatedLog const& log) override final;
  Children children() const override final;
  bool remove(NodeShPtr node) override final;

protected:
  SimpleNode(Utils::WorkerThreadsShPtr workers, Type type, Name name, TrimFields trimFields):
    Node{ std::move(workers), std::move(type), std::move(name), std::move(trimFields) }
  { }

private:
  NodeShPtr addImpl(NodePtr node) override final;
  virtual bool matches(AnnotatedLog const& log) const = 0;

  void insertToChildren(AnnotatedLog const& log);
  //void insertToChild(NodeShPtr const& child, AnnotatedLog const& log);
  void passAllLogsToChild(NodeShPtr child);

  using Lock = std::lock_guard<std::mutex>;
  mutable std::mutex mutex_;
  Children children_;
};

}
