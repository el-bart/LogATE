#pragma once
#include "CursATE/Screen/detail/FilterWindows.hpp"
#include "CursATE/Screen/detail/Marks.hpp"
#include "CursATE/Screen/Search.hpp"
#include "LogATE/Tree/FilterFactory.hpp"
#include "LogATE/Net/Port.hpp"
#include <But/Exception.hpp>
#include <atomic>

namespace CursATE::Screen
{

class LogList final
{
public:
  BUT_DEFINE_EXCEPTION(InvalidMark, ::But::Exception, "invalid mark");

  LogList(LogATE::Utils::WorkerThreadsShPtr workers,
          std::function<size_t()> inputErrors,
          std::function<std::string(LogATE::Log const&)> log2str,
          LogATE::Tree::Node::TrimFields const& trimFields);

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
  void processSearchResult(Search::Result const& result);
  void pipeLogsToHost();
  void createMark();
  void gotoMarkLocal();
  void gotoMarkAbsolute();

  std::atomic<bool> quit_{false};
  LogATE::Utils::WorkerThreadsShPtr workers_;
  std::function<std::string(LogATE::Log const&)> log2str_;
  Search search_;
  LogATE::Tree::FilterFactory filterFactory_;
  detail::FilterWindows filterWindows_;
  detail::Marks marks_;
  const LogATE::Tree::NodeShPtr root_;
  LogATE::Tree::NodeShPtr currentNode_;
  But::NotNullShared<Curses::ScrolableWindow> currentWindow_;
};

}
