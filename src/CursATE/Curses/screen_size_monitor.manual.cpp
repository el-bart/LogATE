#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/ScreenSize.hpp"
#include "CursATE/Curses/ScreenPosition.hpp"

int main()
{
  const CursATE::Curses::Init init;
  for(auto i=0; ; ++i)
  {
    CursATE::Curses::ScreenSize ss{stdscr};
    mvprintw(0, 0, "iteration %d: screen is %d rows and %d cols", i, ss.rows_.value_, ss.columns_.value_);
    clrtoeol();

    CursATE::Curses::ScreenPosition sp{stdscr};
    mvprintw(1, 0, "iteration %d: last cursor position was %d row and %d col", i, sp.row_.value_, sp.column_.value_);
    clrtoeol();

    refresh();
    if( getch() == 'q' )
      break;
  }
}
