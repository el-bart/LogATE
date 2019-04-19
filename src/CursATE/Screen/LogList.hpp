#pragma once
#include "CursATE/Screen/detail/FilterWindows.hpp"
#include "LogATE/Tree/FilterFactory.hpp"
#include <But/Optional.hpp>
#include <atomic>

namespace CursATE::Screen
{

class LogList final
{
public:
  LogList();

  LogList(LogList const&) = delete;
  LogList& operator=(LogList const&) = delete;
  LogList(LogList&&) = delete;
  LogList& operator=(LogList&&) = delete;

  void run();
  void stop() { quit_ = true; }
  auto root() const { return root_; }

private:
  But::Optional<int> getChar() const;
  void reactOnKey(int ch);

  std::atomic<bool> quit_{false};
  LogATE::Tree::FilterFactory filterFactory_;
  detail::FilterWindows filterWindows_;
  LogATE::Tree::NodeShPtr root_;
  But::NotNullShared<Curses::ScrolableWindow> currentWindow_;
};

}
