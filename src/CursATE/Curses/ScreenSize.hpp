#pragma once
#include <ncurses.h>
#include <But/Mpl/FreeOperators.hpp>

namespace CursATE::Curses
{

struct Columns { int value_{0}; };
BUT_MPL_FREE_OPERATORS_COMPARE(Columns, .value_)

struct Rows { int value_{0}; };
BUT_MPL_FREE_OPERATORS_COMPARE(Rows, .value_)

struct ScreenSize
{
  explicit ScreenSize(decltype(stdscr) screen = stdscr)
  {
    getmaxyx(screen, rows_.value_, columns_.value_);
  }

  Columns columns_;
  Rows rows_;
};

}
