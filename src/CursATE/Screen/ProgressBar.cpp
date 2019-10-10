#include "CursATE/Screen/ProgressBar.hpp"
#include "CursATE/Screen/detail/formatAsPercentage.hpp"
#include "CursATE/Curses/Window.hpp"
#include "CursATE/Curses/getChar.hpp"

using CursATE::Curses::Window;
using CursATE::Curses::ScreenSize;
using CursATE::Curses::ScreenPosition;
using CursATE::Curses::Row;
using CursATE::Curses::Column;
using CursATE::Curses::Rows;
using CursATE::Curses::Columns;

namespace CursATE::Screen
{

namespace
{
constexpr auto g_minSize = 10u;
constexpr auto g_delta = 1u;

auto screenSize(const unsigned delta)
{
  const auto minSize = g_minSize + delta;
  auto ss = ScreenSize::global();

  if( static_cast<unsigned>(ss.rows_.value_) < minSize )
    BUT_THROW(ProgressBar::WindowTooSmall, "need at least " << minSize << "x" << minSize << " window size");
  ss.rows_.value_ = 2 + 3;

  if( static_cast<unsigned>(ss.columns_.value_) < minSize )
    BUT_THROW(ProgressBar::WindowTooSmall, "need at least " << minSize << "x" << minSize << " window size");
  ss.columns_.value_ -= 2*delta;

  return ss;
}


void printProgress(Window& win, const ScreenPosition uasp, ProgressBar::Monitor const& monitor)
{
  const auto processed = monitor.processed();
  const auto totalSize = monitor.totalSize();
  const auto progress = totalSize ? ( static_cast<double>(processed) / totalSize ) : 0.0;
  mvwprintw(win.get(), uasp.row_.value_+0, uasp.column_.value_, "progress: %s", detail::formatAsPercentage(progress).c_str());
  mvwprintw(win.get(), uasp.row_.value_+1, uasp.column_.value_, "done: %lu / %lu", processed, totalSize);
  mvwprintw(win.get(), uasp.row_.value_+2, uasp.column_.value_, "press 'q' to abort search");
}
}


bool ProgressBar::process()
{
  if( monitor_->aborted() )
    return false;

  Window win{ ScreenPosition{Row{g_delta}, Column{g_delta}}, screenSize(g_delta), Window::Boxed::True };
  const auto uasp = win.userAreaStartPosition();
  while(true)
  {
    printProgress(win, uasp, *monitor_);
    win.refresh();
    waitForKey();
    if( monitor_->aborted() )
      return false;
    if( monitor_->done() )
      return true;
  }
}


void ProgressBar::waitForKey()
{
  const auto ch = Curses::getChar( std::chrono::milliseconds{100} );
  if(not ch)
    return;
  if(*ch == 'q')
    monitor_->abort();
}

}
