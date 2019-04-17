#include "CursATE/Curses/Field/Input.hpp"
#include "CursATE/Curses/Field/detail/resizePadded.hpp"

namespace CursATE::Curses::Field
{

FieldSize size(Input const& i)
{
  return FieldSize{ i.label_.size(), i.value_.size() };
}

void draw(Window& win, const ScreenPosition sp, const FieldSize fs, Input const& i, const bool selected)
{
  const auto markAttr = A_REVERSE;
  const auto commonAttr = A_UNDERLINE;

  mvwprintw(win.get(), sp.row_.value_, sp.column_.value_, "%s ", detail::resizePadded(i.label_, fs.label_, 0).c_str() );

  auto attr = commonAttr;
  if(selected)
    attr |= markAttr;
  wattr_on( win.get(), attr, nullptr );

  wprintw(win.get(), "%s", detail::resizePadded(i.value_, fs.value_, i.cursorPosition_).c_str() );

  wattr_off( win.get(), attr, nullptr );
}

}
