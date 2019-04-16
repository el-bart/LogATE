#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/Curses/Form.hpp"

using namespace CursATE::Curses;

int main()
{
  const Init init;
  set(CursorVisibility::Invisible);
  auto form = makeForm( Field::Input{"txt"},
                        Field::Radio{"radio", {"foo/bar", "narf"}},
                        Field::Button{"ok"},
                        Field::Button{"cancel"} );

  do
  {
    const auto res = form.process();
    clear();
    refresh();

    mvwprintw( stdscr, 1, 1, "user input:      %s", res[0].c_str() );
    mvwprintw( stdscr, 2, 1, "radio selection: %s", res[1].c_str() );
    mvwprintw( stdscr, 3, 1, "OK button:       %s", res[2].c_str() );
    mvwprintw( stdscr, 3, 1, "cancel button:   %s", res[3].c_str() );
    refresh();
  }
  while( getch() != 'q' );
}
