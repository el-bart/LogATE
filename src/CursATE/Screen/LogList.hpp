#pragma once
#include "CursATE/Screen/detail/FilterWindows.hpp"
#include "CursATE/Screen/Search.hpp"
#include "LogATE/Tree/FilterFactory.hpp"
#include <But/Optional.hpp>
#include <atomic>

namespace CursATE::Screen
{

class LogList final
{
public:
  explicit LogList(LogATE::Utils::WorkerThreadsShPtr workers);

  LogList(LogList const&) = delete;
  LogList& operator=(LogList const&) = delete;
  LogList(LogList&&) = delete;
  LogList& operator=(LogList&&) = delete;

  void run();
  void stop() { quit_ = true; }
  auto root() const { return root_; }

private:
  void reactOnKey(int ch);
  void processQuitProgram();
  void processFilterTree();
  void processLogEntry();
  void centerAllChildrenAroundCurrentLog();
  void centerAllNodesAroundCurrentLog();
  void centerAroundLogSelection(LogATE::Tree::NodeShPtr node);
  void centerAroundLog(LogATE::Tree::NodeShPtr node, LogATE::SequenceNumber sn);
  void processSearch();

  std::atomic<bool> quit_{false};
  Search search_;
  LogATE::Tree::FilterFactory filterFactory_;
  detail::FilterWindows filterWindows_;
  const LogATE::Tree::NodeShPtr root_;
  LogATE::Tree::NodeShPtr currentNode_;
  But::NotNullShared<Curses::ScrolableWindow> currentWindow_;
};

}
