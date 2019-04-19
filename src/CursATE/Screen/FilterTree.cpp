#include "CursATE/Screen/FilterTree.hpp"
#include "CursATE/Screen/detail/FilterTreeDataSource.hpp"
#include "CursATE/Curses/ScrolableWindow.hpp"
#include "CursATE/Curses/ctrl.hpp"

using CursATE::Curses::Window;
using CursATE::Curses::DataSource;
using CursATE::Curses::ScrolableWindow;
using CursATE::Curses::ScreenSize;
using CursATE::Curses::ScreenPosition;
using CursATE::Curses::ctrl;
using CursATE::Curses::Row;
using CursATE::Curses::Column;

namespace CursATE::Screen
{

FilterTree::FilterTree(LogATE::Tree::NodeShPtr root):
  root_{ std::move(root) }
{ }


namespace
{
auto smallerScreenSize()
{
  auto ss = ScreenSize::global();
  if(ss.rows_.value_ > 1+2+2)
    ss.rows_.value_ -= 2;
  if(ss.columns_.value_ > 1+2+2)
    ss.columns_.value_ -= 2;
  return ss;
}

auto navigate(ScrolableWindow& win, const DataSource::Id initialSelection)
{
  win.select(initialSelection);
  while(true)
  {
    win.refresh();
    switch( getch() )
    {
      case 10:
      case KEY_ENTER: return win.currentSelection();
      case 'q': return initialSelection;

      case KEY_UP:    win.selectUp(); break;
      case KEY_DOWN:  win.selectDown(); break;
      case KEY_LEFT:  win.scrollLeft(); break;
      case KEY_RIGHT: win.scrollRight(); break;

      case KEY_PPAGE: win.selectPageUp(); break;
      case KEY_NPAGE: win.selectPageDown(); break;

      case ctrl(KEY_HOME): win.selectFirst(); break;
      case ctrl(KEY_END):  win.selectLast(); break;
      case KEY_HOME: win.scrollToLineBegin(); break;
      case KEY_END:  win.scrollToLineEnd(); break;

      // TODO: searching by string?
      // TODO: moving to a log with a given ID?
      // TODO: move selection to screen begin/center/end
    }
  }
}
}


LogATE::Tree::NodeShPtr FilterTree::selectNext(LogATE::Tree::NodeShPtr const& current)
{
  const auto ds = But::makeSharedNN<detail::FilterTreeDataSource>(root_);
  const auto sp = ScreenPosition{ Row{1}, Column{1} };
  const auto ss = smallerScreenSize();
  ScrolableWindow win{ds, sp, ss, Window::Boxed::True};
  const auto id = ds->node2id(current);
  const auto newId = navigate(win, id);
  return ds->id2node(newId);
}

}
