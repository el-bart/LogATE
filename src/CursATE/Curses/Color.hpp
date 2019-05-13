#pragma once
#include <ncurses.h>

namespace CursATE::Curses
{

enum class Color
{
  Black  = COLOR_BLACK,
  Red    = COLOR_RED,
  Green  = COLOR_GREEN,
  Yellow = COLOR_YELLOW,
  Blue   = COLOR_BLUE,
  Magent = COLOR_MAGENTA,
  Cyan   = COLOR_CYAN,
  White  = COLOR_WHITE
};

inline auto operator<(const Color lhs, const Color rhs)
{
  return static_cast<int>(lhs) < static_cast<int>(rhs);
}

}
