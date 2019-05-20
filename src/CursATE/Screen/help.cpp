#include "CursATE/Screen/help.hpp"
#include "CursATE/Curses/ctrl.hpp"
#include "CursATE/Curses/ScrolableWindow.hpp"
#include "CursATE/Screen/detail/ConstStringDataSource.hpp"
#include "CursATE/Screen/detail/formatAsPercentage.hpp"

using CursATE::Curses::Window;
using CursATE::Curses::ScreenSize;
using CursATE::Curses::ScreenPosition;
using CursATE::Curses::Row;
using CursATE::Curses::Column;
using CursATE::Curses::ScrolableWindow;
using CursATE::Curses::ctrl;

namespace CursATE::Screen
{

namespace
{
auto options()
{
  std::vector<std::string> out{
      "CursATE - ncurses interface to LogATE",
      "",
      ""
    };
  return out;
}


auto smallerScreenSize()
{
  constexpr auto delta = 2;
  auto ss = ScreenSize::global();
  if(ss.rows_.value_ > 1+2*delta)
    ss.rows_.value_ -= 2*delta;
  if(ss.columns_.value_ > 1+2*delta)
    ss.columns_.value_ -= 2*delta;
  return ss;
}


template<typename Win, typename DS>
void navigate(Win& win, DS const& ds)
{
  (void)ds;     // TODO: use for searching
  while(true)
  {
    win.refresh();
    switch( getch() )
    {
      case 'q': return;

      case KEY_UP:    win.selectUp(); break;
      case KEY_DOWN:  win.selectDown(); break;
      case KEY_LEFT:  win.scrollLeft(); break;
      case KEY_RIGHT: win.scrollRight(); break;

      case KEY_PPAGE: win.selectPageUp(); break;
      case KEY_NPAGE: win.selectPageDown(); break;

      case KEY_HOME: win.selectFirst(); break;
      case KEY_END:  win.selectLast(); break;
      case ctrl(KEY_HOME): win.scrollToLineBegin(); break;
      case ctrl(KEY_END):  win.scrollToLineEnd(); break;

      case 'H': win.selectFirstVisible(); break;
      case 'M': win.selectMiddleVisible(); break;
      case 'L': win.selectLastVisible(); break;

      // TODO: searching by string?
    }
  }
}
}

void help()
{
  const auto sp = ScreenPosition{ Row{2}, Column{2} };
  const auto ss = smallerScreenSize();
  const auto ds = But::makeSharedNN<detail::ConstStringDataSource>( options() );
  auto status = [ds](const size_t pos) { return detail::nOFmWithPercent(pos, ds->size()); };
  ScrolableWindow win{ ds, sp, ss, Window::Boxed::True, std::move(status) };
  navigate(win, *ds);
}

}
