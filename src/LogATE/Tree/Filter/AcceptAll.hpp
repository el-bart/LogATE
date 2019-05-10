#pragma once
#include "LogATE/Tree/SimpleNode.hpp"
#include <regex>

namespace LogATE::Tree::Filter
{

class AcceptAll: public SimpleNode
{
public:
  AcceptAll(Utils::WorkerThreadsShPtr workers, Name name, TrimFields tf = TrimFields{});

private:
  bool matches(AnnotatedLog const& log) const override;
};

}
