#pragma once
#include "LogATE/Tree/SimpleNode.hpp"

namespace LogATE::Tree::Filter
{

class To: public SimpleNode
{
public:
  explicit To(Utils::WorkerThreadsShPtr workers, Name name, Log::Key edge):
    SimpleNode{ std::move(workers), Type{"To"}, std::move(name), {} },
    edge_{ std::move(edge) }
  { }

private:
  bool matches(AnnotatedLog const& log) const override
  {
    return log.log().key() <= edge_;
  }

  const Log::Key edge_;
};

}
