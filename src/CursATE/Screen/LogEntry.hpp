#pragma once
#include "CursATE/Screen/detail/FilterWindows.hpp"
#include "LogATE/Tree/Node.hpp"

namespace CursATE::Screen
{

class LogEntry final
{
public:
  LogEntry(LogATE::Tree::NodeShPtr node, LogATE::Log log):
    node_{ std::move(node) },
    log_{ std::move(log) }
  { }

  LogEntry(LogEntry const&) = delete;
  LogEntry& operator=(LogEntry const&) = delete;
  LogEntry(LogEntry&&) = delete;
  LogEntry& operator=(LogEntry&&) = delete;

  std::shared_ptr<LogATE::Tree::Node> process();

private:
  template<typename Win, typename DS>
  std::shared_ptr<LogATE::Tree::Node> navigate(Win& win, DS const& ds);
  template<typename DS>
  LogATE::Tree::NodeShPtr createFilterBasedOnSelection(DS const& ds, Curses::DataSource::Id id) const;

  LogATE::Tree::NodeShPtr node_;
  const LogATE::Log log_;
};

}
