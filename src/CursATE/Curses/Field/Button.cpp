#include "CursATE/Curses/Field/Button.hpp"
#include <ncurses.h>

namespace CursATE::Curses::Field
{

FieldSize size(Button const& b)
{
  return FieldSize{ b.label_.size() + 2*2, 0 };
}

void draw(Window& win, const ScreenPosition sp, const FieldSize fs, Button const& b, const bool selected)
{
  (void)fs;
  const auto markAttr = A_REVERSE;
  if(selected)
    wattr_on( win.get(), markAttr, nullptr);

  mvwprintw(win.get(), sp.row_.value_, sp.column_.value_, "[ %s ]", b.label_.c_str());

  if(selected)
    wattr_off( win.get(), markAttr, nullptr);
}

}
