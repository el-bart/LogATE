#include "CursATE/Screen/help.hpp"
#include "CursATE/Curses/ctrl.hpp"
#include "CursATE/Curses/ScrolableWindow.hpp"
#include "CursATE/Screen/detail/ConstStringDataSource.hpp"
#include "CursATE/Screen/detail/formatAsPercentage.hpp"
#include "CursATE/Screen/detail/smallerScreenSize.hpp"

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
      ">> CursATE - ncurses interface to LogATE <<",
      "TODO: version / hash / branch",  // TODO!!!
      "very helpful help screen",
      "",
      "",
      "### common keys",
      "  arrow keys   - navigate window",
      "  home/end     - select first/last entry",
      "  page up/down - move one page up/down",
      "  ^home/^end   - begin/end of line",
      "  H            - select Highest visible entry",
      "  M            - select Middle (center) visible entry",
      "  L            - select Lowest visible entry",
      "  q            - Quit current window",
      "  F1/h         - helpful help screen full of help",
      "",
      "",
      "### log list window",
      "  q            - Quit program (requires confirmation, to avoid WTF-class issues)",
      "  j            - Jump (center) all child filters to current log (or closest one, if not present directly)",
      "  J            - the same as 'j', but affecting all filters, globally",
      "  f            - go to log view (Filter creation is possible there)",
      "  t            - open filter Tree window",
      "  enter        - alias for 't' key",
      "  /            - open search key/value window - forward search",
      "  ?            - open search key/value window - backward search",
      "  n            - repeat last search forward (select Next match)",
      "  N            - repeat last search backward",
      "",
      "",
      "### filter tree window",
      "  enter        - switch to selected filter",
      "",
      "",
      "### log entry detailed view window",
      "  i            - Inspect given field in a more human readable preview window",
      "  f            - create new Filter, based on a current selection",
      "  enter        - alias for 'f' key"
    };
  return out;
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
      case KEY_F(1):
      case 'h':
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
  const auto ss = detail::smallerScreenSize(2);
  const auto ds = But::makeSharedNN<detail::ConstStringDataSource>( options() );
  auto status = [ds](const size_t pos) { return detail::nOFmWithPercent(pos, ds->size()); };
  ScrolableWindow win{ ds, sp, ss, Window::Boxed::True, std::move(status) };
  navigate(win, *ds);
}

}
