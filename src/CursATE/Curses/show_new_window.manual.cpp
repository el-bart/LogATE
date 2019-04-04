#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/Window.hpp"

using namespace CursATE::Curses;

int main()
{
  const Init init;
  Window win{ ScreenPosition{Row{2}, Column{10}}, ScreenSize{Rows{5}, Columns{60}}, Window::Boxed::True };

  for(auto i=0; ; ++i)
  {
    CursATE::Curses::ScreenSize ss{win.window_};
    mvwprintw(win.window_, 1, 1, "iteration %d: screen is %d rows and %d cols", i, ss.rows_.value_, ss.columns_.value_);

    CursATE::Curses::ScreenPosition sp{win.window_};
    mvwprintw(win.window_, 2, 1, "iteration %d: last cursor position was %d row and %d col", i, sp.row_.value_, sp.column_.value_);

    win.refresh();
    if( getch() == 'q' )
      break;
  }
}
