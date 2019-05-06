#pragma once
#include "LogATE/Tree/SimpleNode.hpp"

namespace LogATE::Tree::Filter
{

class From: public SimpleNode
{
public:
  explicit From(Utils::WorkerThreadsShPtr workers, Name name, const SequenceNumber edge):
    SimpleNode{ std::move(workers), Type{"From"}, std::move(name), {} },
    edge_{edge}
  { }

private:
  bool matches(Log const& log) const override
  {
    return log.sn_ >= edge_;
  }

  const SequenceNumber edge_;
};

}
