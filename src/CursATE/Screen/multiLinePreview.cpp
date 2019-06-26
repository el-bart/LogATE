#include "CursATE/Screen/help.hpp"
#include "CursATE/Curses/ctrl.hpp"
#include "CursATE/Curses/ScrolableWindow.hpp"
#include "CursATE/Screen/detail/ConstStringDataSource.hpp"
#include "CursATE/Screen/detail/formatAsPercentage.hpp"
#include "CursATE/Screen/detail/smallerScreenSize.hpp"
#include "CursATE/Screen/detail/splitIntoLines.hpp"
#include "CursATE/Screen/help.hpp"
#include "LogATE/Utils/PrintableStringConverter.hpp"

using CursATE::Curses::Window;
using CursATE::Curses::ScreenSize;
using CursATE::Curses::ScreenPosition;
using CursATE::Curses::Row;
using CursATE::Curses::Column;
using CursATE::Curses::ScrolableWindow;
using CursATE::Curses::ctrl;
using LogATE::Utils::PrintableStringConverter;

namespace CursATE::Screen
{

namespace
{
template<typename Win>
void navigate(Win& win)
{
  while(true)
  {
    win.refresh();
    switch( getch() )
    {
      case Curses::escapeKey:
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

      case 'h':
      case KEY_F(1): help(); break;

      // TODO: searching by string?
    }
  }
}
}

void multiLinePreview(std::string const& in)
{
  const auto sp = ScreenPosition{ Row{2}, Column{2} };
  const auto ss = detail::smallerScreenSize(2);
  const auto psc = PrintableStringConverter{};
  const auto ds = But::makeSharedNN<detail::ConstStringDataSource>( detail::splitIntoLines( psc(in) , ss.columns_.value_) );
  auto status = [ds](const size_t pos) { return detail::nOFmWithPercent(pos, ds->size()); };
  ScrolableWindow win{ ds, sp, ss, Window::Boxed::True, std::move(status) };
  navigate(win);
}

}
