#pragma once
#include "LogATE/Tree/Node.hpp"
#include <string>

namespace CursATE::Screen
{

class Search final
{
public:
  explicit Search(LogATE::Utils::WorkerThreadsShPtr workers):
    workers_{ std::move(workers) }
  { }

  Search(Search const&) = delete;
  Search& operator=(Search const&) = delete;
  Search(Search&&) = delete;
  Search& operator=(Search&&) = delete;

  But::Optional<LogATE::SequenceNumber> process(LogATE::Tree::NodeShPtr node, LogATE::SequenceNumber currentSelection);

private:
  bool updateSearchPattern();
  But::Optional<LogATE::SequenceNumber> triggerSearch(LogATE::Tree::NodeShPtr node, LogATE::SequenceNumber currentSelection);

  LogATE::Utils::WorkerThreadsShPtr workers_;
  std::string keyQuery_;
  std::string valueQuery_;
};

}
