#include "CursATE/Screen/displayError.hpp"
#include "CursATE/Curses/Window.hpp"
#include <ncurses.h>

using CursATE::Curses::Window;
using CursATE::Curses::ScreenSize;
using CursATE::Curses::ScreenPosition;
using CursATE::Curses::Row;
using CursATE::Curses::Column;
using CursATE::Curses::Rows;
using CursATE::Curses::Columns;
using CursATE::Curses::Color;

namespace CursATE::Screen
{

namespace
{
constexpr auto g_minSize = 10u;
constexpr auto g_delta = 1u;

auto screenSize(const unsigned delta, const size_t messageLines)
{
  const auto minSize = g_minSize + delta;
  auto ss = ScreenSize::global();

  const auto expectedLines = 2*1 + 2 + messageLines;
  if( static_cast<unsigned>(ss.rows_.value_) < expectedLines )
    BUT_THROW(ErrorWindowTooSmall, "need at least " << expectedLines << "x" << minSize << " window size");
  ss.rows_.value_ = expectedLines;

  if( static_cast<unsigned>(ss.columns_.value_) < minSize + 2*delta )
    BUT_THROW(ErrorWindowTooSmall, "need at least " << expectedLines << "x" << minSize << " window size");
  ss.columns_.value_ -= 2*delta;

  return ss;
}
}


void displayError(std::vector<std::string> const& lines)
{
  Window win{ ScreenPosition{Row{g_delta}, Column{g_delta}}, screenSize(g_delta, lines.size()), Window::Boxed::True };
  win.colors(Color::Red, Color::White);
  const auto uasp = win.userAreaStartPosition();

  mvwprintw(win.get(), uasp.row_.value_, uasp.column_.value_, "[ ERROR ]");
  for(auto i=0u; i<lines.size(); ++i)
    mvwprintw(win.get(), uasp.row_.value_+2+i, uasp.column_.value_, "%s", lines[i].c_str());

  win.refresh();
  getch();
}

}
