#include "CursATE/Curses/Window.hpp"
#include "CursATE/Curses/ColorGuard.hpp"
#include <ncurses.h>

namespace CursATE::Curses
{

Window::Window(const ScreenPosition sp, const ScreenSize ss, const Boxed boxed):
  boxed_{boxed},
  window_{ newwin(ss.rows_.value_, ss.columns_.value_,
                  sp.row_.value_,  sp.column_.value_) }
{
  if(not window_)
    BUT_THROW(FailedToCreateWindow, "nullptr received");
  keypad(window_, TRUE);   // enable funciton keys, arrows, etc...
}

Window::~Window()
{
  switch(boxed_)
  {
    case Boxed::True: wborder(window_, ' ', ' ', ' ',' ',' ',' ',' ',' '); break;
    case Boxed::False: break;
  }
  clear();
  wrefresh(window_);
  delwin(window_);
}

void Window::clear() const
{
  wclear(window_);
}

void Window::refresh() const
{
  switch(boxed_)
  {
    case Boxed::True: box(window_, 0, 0); break;
    case Boxed::False: break;
  }
  wrefresh(window_);
}

ScreenPosition Window::userAreaStartPosition() const
{
  if( boxed() )
    return ScreenPosition{ Row{1}, Column{1} };
  return ScreenPosition{ Row{0}, Column{0} };
}


ScreenSize Window::userAreaSize() const
{
  const auto ss = size();
  if( boxed() )
    return ScreenSize{ Rows{std::max(ss.rows_.value_-2, 0)}, Columns{std::max(ss.columns_.value_-2, 0)} };
  return ss;
}

void Window::colors(const Color background, const Color font)
{
  const auto cp = ColorGuard::colorPairId(font, background);
  wbkgd(window_, COLOR_PAIR(cp));
}

}
