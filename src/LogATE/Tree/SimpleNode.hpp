#pragma once
#include "LogATE/Tree/Node.hpp"

namespace LogATE::Tree
{

class SimpleNode: public Node
{
public:
  SequenceNumber insert(Log log) override;
  Children children() const override;
  void add(NodeShPtr node) override;

protected:
  SimpleNode(Type type, Name name, TrimFields trimFields):
    Node{ std::move(type), std::move(name), std::move(trimFields) }
  { }

private:
  virtual bool matches(Log const& log) const = 0;

  mutable std::mutex mutex_;
  Children children_;
};

}
