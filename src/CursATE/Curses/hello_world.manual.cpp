#include "CursATE/Curses/Init.hpp"

int main()
{
  const CursATE::Curses::Init init;
  printw("hello world");
  refresh();
  getch();
}
