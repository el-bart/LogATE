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
  LogList(LogATE::Utils::WorkerThreadsShPtr workers,
          std::function<size_t()> inputErrors,
          std::function<std::string(LogATE::Log const&)> log2str);

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
  void centerAroundLog(LogATE::Tree::NodeShPtr node, LogATE::Log::Key const& key);
  void processSearch(Search::Direction dir);
  void processSearchAgain(Search::Direction dir);

  std::atomic<bool> quit_{false};
  LogATE::Utils::WorkerThreadsShPtr workers_;
  Search search_;
  LogATE::Tree::FilterFactory filterFactory_;
  detail::FilterWindows filterWindows_;
  const LogATE::Tree::NodeShPtr root_;
  LogATE::Tree::NodeShPtr currentNode_;
  But::NotNullShared<Curses::ScrolableWindow> currentWindow_;
};

}
