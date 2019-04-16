#include "CursATE/Curses/Field/Button.hpp"
#include <ncurses.h>

namespace CursATE::Curses::Field
{

FieldSize size(Button const& b)
{
  return FieldSize{ b.label_.size() + 2, 0 };
}

void draw(Window& win, const ScreenPosition sp, const FieldSize fs, Button const& b)
{
  (void)win;
  (void)sp;
  (void)fs;
  (void)b;
  // TODO...
}

}
