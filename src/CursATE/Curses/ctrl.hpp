#pragma once
#include <ncurses.h>

namespace CursATE::Curses
{

inline constexpr auto ctrl(int k) { return k & 0x1f; }

}
