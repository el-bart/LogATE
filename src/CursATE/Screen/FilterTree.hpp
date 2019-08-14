#pragma once
#include "CursATE/Screen/detail/FilterWindows.hpp"
#include "LogATE/Tree/Node.hpp"

namespace CursATE::Screen
{

class FilterTree final
{
public:
  FilterTree(LogATE::Utils::WorkerThreadsShPtr workers, LogATE::Tree::NodeShPtr root);

  FilterTree(FilterTree const&) = delete;
  FilterTree& operator=(FilterTree const&) = delete;
  FilterTree(FilterTree&&) = delete;
  FilterTree& operator=(FilterTree&&) = delete;

  LogATE::Tree::NodeShPtr selectNext(LogATE::Tree::NodeShPtr const& current);

private:
  bool deleteNode(LogATE::Tree::NodeShPtr const& selected);

  LogATE::Utils::WorkerThreadsShPtr workers_;
  LogATE::Tree::NodeShPtr root_;
};

}
