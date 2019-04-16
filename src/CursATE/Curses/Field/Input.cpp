#include "CursATE/Curses/Field/Input.hpp"

namespace CursATE::Curses::Field
{

FieldSize size(Input const& i)
{
  return FieldSize{ i.label_.size(), i.value_.size() };
}

void draw(Window& win, const ScreenPosition sp, const FieldSize fs, Input const& i, const bool selected)
{
  (void)win;
  (void)sp;
  (void)fs;
  (void)i;
  (void)selected;
  // TODO
}

}
