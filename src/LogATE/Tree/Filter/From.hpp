#pragma once
#include "LogATE/Tree/SimpleNode.hpp"

namespace LogATE::Tree::Filter
{

class From: public SimpleNode
{
public:
  explicit From(Utils::WorkerThreadsShPtr workers, Name name, Log::Key edge):
    SimpleNode{ std::move(workers), Type{"From"}, std::move(name), {} },
    edge_{ std::move(edge) }
  { }

private:
  bool matches(AnnotatedLog const& log) const override
  {
    return log.log().key() >= edge_;
  }

  const Log::Key edge_;
};

}
