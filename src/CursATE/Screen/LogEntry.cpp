#include "CursATE/Screen/LogEntry.hpp"
#include "CursATE/Screen/detail/LogEntryDataSource.hpp"
#include "CursATE/Curses/Form.hpp"
#include "CursATE/Curses/Field/Button.hpp"
#include "CursATE/Curses/Field/Input.hpp"
#include "CursATE/Curses/Field/Radio.hpp"
#include "CursATE/Curses/ScrolableWindow.hpp"
#include "CursATE/Curses/ctrl.hpp"

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
}

std::shared_ptr<LogATE::Tree::Node> LogEntry::process()
{
  const auto sp = ScreenPosition{ Row{1}, Column{1} };
  const auto ss = smallerScreenSize();
  const auto ds = But::makeSharedNN<detail::LogEntryDataSource>(log_);
  ScrolableWindow win{ds, sp, ss, Window::Boxed::True};
  return navigate(win, *ds);
}


template<typename Win, typename DS>
std::shared_ptr<LogATE::Tree::Node> LogEntry::navigate(Win& win, DS const& ds)
{
  //win.select( ds.first() );
  while(true)
  {
    win.refresh();
    switch( getch() )
    {
      case 10:
      case KEY_ENTER: return createFilterBasedOnSelection( ds, *win.currentSelection() );
      case 'q': return {};

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


namespace
{
auto supportedFilters()
{
  return std::array<std::string, 3>{{"Grep", "Explode", "AcceptAll"}};
}

But::Optional<std::string> selectFilter()
{
  const auto names = supportedFilters();
  auto form = Form{ KeyShortcuts{
                                  {'g', names[0]},
                                  {'e', names[1]},
                                  {'a', names[2]},
                                  {'c', "cancel"},
                                  {'q', "cancel"}
                                },
                    Button{names[0]},
                    Button{names[1]},
                    Button{names[2]},
                    Button{"cancel"}
                  };
  const auto ret = form.process();
  static_assert( ret.size() == names.size()+1u );
  for(auto i=0u; i<names.size(); ++i)
    if(ret[i] == "true")
      return names[i];
  // cancle
  BUT_ASSERT( *ret.rbegin() == "true" );
  return {};
}
}


template<typename DS>
std::shared_ptr<LogATE::Tree::Node> LogEntry::createFilterBasedOnSelection(DS const& ds, const Curses::DataSource::Id id) const
{
  const auto filterName = selectFilter();
  if(not filterName)
    return {};
  throw 42;
  (void)ds;
  (void)id;
}

}
