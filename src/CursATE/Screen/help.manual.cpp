#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/Screen/help.hpp"

using namespace CursATE::Curses;

int main()
{
  const Init init;
  set(CursorVisibility::Invisible);
  CursATE::Screen::help();
}
