#pragma once
#include <ncurses.h>
#include "CursATE/Curses/Exception.hpp"

namespace CursATE::Curses
{

enum class CursorVisibility
{
  Invisible = 0,
  Normal = 1,
  VeryVisible =2
};

BUT_DEFINE_EXCEPTION(FailedToSetCursorVisibility, Exception, "failed to set cursor visibility");

inline void set(const CursorVisibility cv)
{
  const auto cvn = static_cast<int>(cv);
  if( curs_set(cvn) == ERR )
    BUT_THROW(FailedToSetCursorVisibility, "expected cursor visibility: " << cvn);
}

}
