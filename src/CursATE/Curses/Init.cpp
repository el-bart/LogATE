#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/CursorVisibility.hpp"
#include <ncurses.h>
#include <mutex>

namespace CursATE::Curses
{

namespace
{
std::mutex g_mutex;
auto g_counter{0};
using Lock = std::lock_guard<std::mutex>;
}

Init::Init()
{
  const Lock lock{g_mutex};
  ++g_counter;
  if(g_counter > 1)
    return;

  initscr();  // ncurses init
  if(not has_colors())
    BUT_THROW(ColorsNotSupported, "has_colors() returned false");
  start_color();
  set(CursorVisibility::Normal);
  cbreak();   // ^Z and ^C shall generate signals
  noecho();
  keypad(stdscr, TRUE);   // enable funciton keys, arrows, etc...
  refresh();
}

Init::~Init()
{
  const Lock lock{g_mutex};
  --g_counter;
  if(g_counter > 0)
    return;
  endwin();
}

}
