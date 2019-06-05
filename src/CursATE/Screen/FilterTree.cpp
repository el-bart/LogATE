#include "CursATE/Screen/FilterTree.hpp"
#include "CursATE/Screen/displayError.hpp"
#include "CursATE/Screen/help.hpp"
#include "CursATE/Screen/detail/FilterTreeDataSource.hpp"
#include "CursATE/Screen/detail/formatAsPercentage.hpp"
#include "CursATE/Screen/detail/smallerScreenSize.hpp"
#include "CursATE/Curses/Form.hpp"
#include "CursATE/Curses/Field/Button.hpp"
#include "CursATE/Curses/ScrolableWindow.hpp"
#include "CursATE/Curses/ctrl.hpp"
#include "LogATE/Tree/findParent.hpp"

using CursATE::Curses::Form;
using CursATE::Curses::KeyShortcuts;
using CursATE::Curses::Field::Button;
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
auto confirmDelete()
{
  auto form = Form{
                    KeyShortcuts{
                                  {'d', "delete"},
                                  {'c', "cancel"},
                                  {'q', "cancel"}
                                },
                                Button{"cancel"},
                                Button{"delete"}
                  };
  const auto res = form.process();
  return res[1] == "true";
}

template<typename F>
std::pair<DataSource::Id, bool> navigate(ScrolableWindow& win,
                                         const DataSource::Id initialSelection,
                                         detail::FilterTreeDataSource const& ds,
                                         F&& nodeDeleter)
{
  win.select(initialSelection);
  while(true)
  {
    win.refresh();
    switch( getch() )
    {
      case 10:
      case KEY_ENTER: return std::make_pair( *win.currentSelection(), true );
      case 'q': return std::make_pair(initialSelection, true);

      case 'd':
      case KEY_DC:
           {
             if( not confirmDelete() )
               break;
             const auto now = win.currentSelection();
             if( nodeDeleter( ds.id2node(*now) ) )
               win.selectUp();
             return std::make_pair( *win.currentSelection(), false );
           }

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
      // TODO: moving to a log with a given ID?
    }
  }
}
}

LogATE::Tree::NodeShPtr FilterTree::selectNext(LogATE::Tree::NodeShPtr const& current)
{
  auto selectedNode = current;
  const auto sp = ScreenPosition{ Row{1}, Column{1} };
  const auto ss = detail::smallerScreenSize(1);
  while(true)
  {
    const auto ds = But::makeSharedNN<detail::FilterTreeDataSource>(root_);
    auto status = [ds](const size_t pos) { return detail::nOFmWithPercent(pos, ds->size()); };
    ScrolableWindow win{ds, sp, ss, Window::Boxed::True, std::move(status)};
    const auto id = ds->node2id(selectedNode);
    const auto newId = navigate( win, id, *ds, [&](auto const& node){ return this->deleteNode(node); } );
    selectedNode = ds->id2node(newId.first);
    if(newId.second)
      return selectedNode;
  }
}


bool FilterTree::deleteNode(LogATE::Tree::NodeShPtr const& selected)
{
  try
  {
    if( selected.get() == root_.get() )
      return false;
    auto parent = findParent(root_, selected);
    if(not parent)
      return false;
    return parent->remove(selected);
  }
  catch(std::exception const& ex)
  {
    displayError(ex);
  }
  return false;
}

}
