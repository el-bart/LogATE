#pragma once
#include "CursATE/Curses/ScreenPosition.hpp"
#include "CursATE/Curses/ScreenSize.hpp"
#include "CursATE/Curses/Exception.hpp"
#include "CursATE/Curses/Color.hpp"
#include <ncurses.h>

namespace CursATE::Curses
{

struct Window
{
  BUT_DEFINE_EXCEPTION(FailedToCreateWindow, Exception, "failed to create window");

  enum class Boxed
  {
    False,
    True
  };

  Window(ScreenPosition sp, ScreenSize ss, Boxed boxed);
  ~Window();

  Window(Window const&) = delete;
  Window& operator=(Window const&) = delete;

  Window(Window&&) = delete;
  Window& operator=(Window&&) = delete;

  void clear() const;
  void refresh() const;

  auto size() const { return ScreenSize{window_}; }
  auto position() const { return ScreenPosition{window_}; }
  auto boxed() const { return boxed_ == Boxed::True; }
  auto get() const { return window_; }

  ScreenPosition userAreaStartPosition() const;
  ScreenSize userAreaSize() const;

  void colors(Color background, Color font);

private:
  const Boxed boxed_;
  WINDOW* window_;
};

}
