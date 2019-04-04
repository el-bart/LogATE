#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/ScreenSize.hpp"

int main()
{
  const CursATE::Curses::Init init;
  for(auto i=0; ; ++i)
  {
    CursATE::Curses::ScreenSize ss;
    mvprintw(0, 0, "iteration %d: screen is %d rows and %d cols", i, ss.rows_.value_, ss.columns_.value_);
    refresh();
    if( getch() == 'q' )
      break;
  }
}
