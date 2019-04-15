#include "CursATE/Curses/Init.hpp"
#include <ncurses.h>

int main()
{
  const CursATE::Curses::Init init1;
  printw("hello world");
  const CursATE::Curses::Init init2;    // does nothing
  refresh();
  getch();
}
