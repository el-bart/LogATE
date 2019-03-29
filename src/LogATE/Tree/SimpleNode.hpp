#pragma once
#include "LogATE/Tree/Node.hpp"

namespace LogATE::Tree
{

class SimpleNode: public Node
{
public:
  void insert(Log const& log) override;
  Children children() const override;
  void add(NodePtr node) override;

protected:
  SimpleNode(Type type, Name name, TrimFields trimFields):
    Node{ std::move(type), std::move(name), std::move(trimFields) }
  { }

private:
  virtual bool matches(Log const& log) const = 0;

  using Lock = std::lock_guard<std::mutex>;
  mutable std::mutex mutex_;
  Children children_;
};

}
