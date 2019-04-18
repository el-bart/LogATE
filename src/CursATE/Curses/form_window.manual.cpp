#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/Curses/Form.hpp"

using namespace CursATE::Curses;

auto processForm()
{
  const auto shortcuts = KeyShortcuts{
      {'o', "ok"},
      {'q', "cancel"},
      {'c', "cancel"}
    };
  auto form = makeForm( shortcuts,
                        Field::Input{"txt", "default window text XXX AAA"},
                        Field::Radio{"radio", {"foo/bar", "narf"}},
                        Field::Button{"ok"},
                        Field::Button{"cancel"} );

  return form.process();
}

int main()
{
  const Init init;
  set(CursorVisibility::Invisible);
  const auto res = processForm();

  mvwprintw( stdscr, 1, 1, "user input:      %s", res[0].c_str() );
  mvwprintw( stdscr, 2, 1, "radio selection: %s", res[1].c_str() );
  mvwprintw( stdscr, 3, 1, "OK button:       %s", res[2].c_str() );
  mvwprintw( stdscr, 4, 1, "cancel button:   %s", res[3].c_str() );
  refresh();
  getch();
}
