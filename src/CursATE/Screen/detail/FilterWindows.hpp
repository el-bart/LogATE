#pragma once
#include "LogATE/Tree/Node.hpp"
#include "CursATE/Curses/DataSource.hpp"
#include "CursATE/Curses/ScrolableWindow.hpp"
#include <functional>
#include <vector>

namespace CursATE::Screen::detail
{

class FilterWindows final
{
public:
  FilterWindows(std::function<std::string(LogATE::Log const&)> log2str,
                std::function<size_t()> inputErrors,
                std::function<std::string()> workerThreadsStats);

  But::NotNullShared<Curses::ScrolableWindow> window(LogATE::Tree::NodeShPtr const& node);
  void prune();

private:
  But::NotNullShared<Curses::ScrolableWindow> newWindow(LogATE::Tree::NodeShPtr node) const;

  struct Element
  {
    LogATE::Tree::NodeWeakPtr node_;
    But::NotNullShared<Curses::ScrolableWindow> win_;
  };
  std::vector<Element> entries_;

  const std::function<std::string(LogATE::Log const&)> log2str_;
  const std::function<size_t()> inputErrors_;
  const std::function<std::string()> workerThreadsStats_;
};

}
