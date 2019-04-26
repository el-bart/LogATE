#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/Curses/Form.hpp"

using namespace CursATE::Curses;

auto processForm()
{
  auto form = makeForm( KeyShortcuts{
                          {'i', "txt"},
                          {'1', "txt"},
                          {'2', "txt2"},
                          {'3', "txt3"},
                          {'r', "radio"},
                          {'o', "ok"},
                          {'q', "cancel"},
                          {'c', "cancel"}
                        },
                        Field::Input{"txt", "default window text XXX AAA"},
                        Field::Input{"txt2", "other stuff"},
                        Field::Input{"txt3", "and one more"},
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
  mvwprintw( stdscr, 2, 1, "user input 2:    %s", res[1].c_str() );
  mvwprintw( stdscr, 3, 1, "user input 3:    %s", res[2].c_str() );
  mvwprintw( stdscr, 4, 1, "radio selection: %s", res[3].c_str() );
  mvwprintw( stdscr, 5, 1, "OK button:       %s", res[4].c_str() );
  mvwprintw( stdscr, 6, 1, "cancel button:   %s", res[5].c_str() );
  refresh();
  getch();
}
