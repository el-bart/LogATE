#pragma once
#include "CursATE/Curses/ScreenSize.hpp"

namespace CursATE::Screen::detail
{

inline auto smallerScreenSize(const int delta)
{
  auto ss = Curses::ScreenSize::global();
  if(ss.rows_.value_ > 1+2*delta)
    ss.rows_.value_ -=   2*delta;
  if(ss.columns_.value_ > 1+2*delta)
    ss.columns_.value_ -=   2*delta;
  return ss;
}

}
