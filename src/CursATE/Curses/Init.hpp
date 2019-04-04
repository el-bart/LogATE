#pragma once
#include <ncurses.h>
#include "CursATE/Curses/Exception.hpp"

namespace CursATE::Curses
{

struct Init
{
  BUT_DEFINE_EXCEPTION(ColorsNotSupported, Exception, "colors not supported");

  Init()
  {
    initscr();  // ncurses init
    if(not has_colors())
      BUT_THROW(ColorsNotSupported, "has_colors() returned false");
    start_color();
    cbreak();   // ^Z and ^C shall generate signals
    noecho();
    keypad(stdscr, TRUE);   // enable funciton keys, arrows, etc...
    refresh();
  }

  ~Init()
  {
    endwin();
  }

  Init(Init const&) = delete;
  Init& operator=(Init const&) = delete;

  Init(Init&&) = delete;
  Init& operator=(Init&&) = delete;
};

}
