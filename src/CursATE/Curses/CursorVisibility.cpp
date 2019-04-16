#include "CursATE/Curses/CursorVisibility.hpp"
#include <ncurses.h>

namespace CursATE::Curses
{

namespace
{
auto g_cursorVisibility = CursorVisibility::Normal;
}

void set(const CursorVisibility cv)
{
  const auto cvn = static_cast<int>(cv);
  if( curs_set(cvn) == ERR )
    BUT_THROW(FailedToSetCursorVisibility, "expected cursor visibility: " << cvn);
  g_cursorVisibility = cv;
}


CursorVisibility cursorVisibility()
{
  return g_cursorVisibility;
}

}
