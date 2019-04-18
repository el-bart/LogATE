#pragma once
#include "LogATE/Tree/Node.hpp"
#include "CursATE/Curses/DataSource.hpp"
#include "CursATE/Curses/ScrolableWindow.hpp"
#include <functional>
#include <vector>

namespace CursATE::Screen::detail
{

class FilterWindows
{
public:
  explicit FilterWindows(std::function<std::string(LogATE::Log const&)> log2str);

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
};

}
