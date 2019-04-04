#pragma once
#include <ncurses.h>
#include <But/Mpl/FreeOperators.hpp>
#include <But/assert.hpp>

namespace CursATE::Curses
{

struct Column { int value_{0}; };
BUT_MPL_FREE_OPERATORS_COMPARE(Column, .value_)

struct Row { int value_{0}; };
BUT_MPL_FREE_OPERATORS_COMPARE(Row, .value_)

struct ScreenPosition
{
  explicit ScreenPosition(WINDOW* screen)
  {
    BUT_ASSERT(screen);
    getyx(screen, row_.value_, column_.value_);
  }
  ScreenPosition(const Row row, const Column column):
    row_{row},
    column_{column}
  { }

  Row row_;
  Column column_;
};

}
