#pragma once
#include <ncurses.h>

namespace CursATE::Curses
{

struct Init
{
  Init()
  {
    initscr();  // ncurses init
    cbreak();   // ^Z and ^C shall generate signals
    noecho();
    keypad(stdscr, TRUE);   // enable funciton keys, arrows, etc...
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
