#pragma once
#include <ncurses.h>
#include <But/Mpl/FreeOperators.hpp>

namespace CursATE::Curses
{

struct Column { int value_{0}; };
BUT_MPL_FREE_OPERATORS_COMPARE(Column, .value_)

struct Row { int value_{0}; };
BUT_MPL_FREE_OPERATORS_COMPARE(Row, .value_)

struct ScreenPosition
{
  explicit ScreenPosition(decltype(stdscr) screen = stdscr)
  {
    getyx(screen, row_.value_, column_.value_);
  }

  Column column_;
  Row row_;
};

}
