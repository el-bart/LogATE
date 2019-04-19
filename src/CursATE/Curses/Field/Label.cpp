#include "CursATE/Curses/Field/Label.hpp"
#include <ncurses.h>

namespace CursATE::Curses::Field
{

FieldSize size(Label const& b)
{
  return FieldSize{ b.label_.size() };
}

void draw(Window& win, const ScreenPosition sp, const FieldSize fs, Label const& b, const bool selected)
{
  (void)fs;
  (void)selected;
  mvwprintw(win.get(), sp.row_.value_, sp.column_.value_, "%s", b.label_.c_str());
}

}
