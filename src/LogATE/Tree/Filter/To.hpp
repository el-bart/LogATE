#pragma once
#include "LogATE/Tree/SimpleNode.hpp"

namespace LogATE::Tree::Filter
{

class To: public SimpleNode
{
public:
  explicit To(Utils::WorkerThreadsShPtr workers, Name name, const SequenceNumber edge):
    SimpleNode{ std::move(workers), Type{"To"}, std::move(name), {} },
    edge_{edge}
  { }

private:
  bool matches(AnnotatedLog const& log) const override
  {
    return log.log_.sequenceNumber() <= edge_;
  }

  const SequenceNumber edge_;
};

}
