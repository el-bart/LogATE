#pragma once
#include <ncurses.h>

namespace CursATE::Curses
{
struct Init
{
  Init()
  {
    initscr();
    cbreak();
    noecho();
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
};
