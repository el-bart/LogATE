#pragma once
#include "LogATE/Tree/Node.hpp"
#include <string>

namespace CursATE::Screen
{

class Search final
{
public:
  enum class Direction
  {
    Forward,
    Backward
  };

  explicit Search(LogATE::Utils::WorkerThreadsShPtr workers):
    workers_{ std::move(workers) }
  { }

  Search(Search const&) = delete;
  Search& operator=(Search const&) = delete;
  Search(Search&&) = delete;
  Search& operator=(Search&&) = delete;

  But::Optional<LogATE::Log::Key> process(LogATE::Tree::NodeShPtr node, LogATE::Log::Key currentSelection, Direction dir);
  But::Optional<LogATE::Log::Key> processAgain(LogATE::Tree::NodeShPtr node, LogATE::Log::Key currentSelection, Direction dir);

private:
  bool updateSearchPattern();
  But::Optional<LogATE::Log::Key> triggerSearch(LogATE::Tree::NodeShPtr node, LogATE::Log::Key currentSelection, Direction dir);

  LogATE::Utils::WorkerThreadsShPtr workers_;
  std::string keyQuery_;
  std::string valueQuery_;
};

}
