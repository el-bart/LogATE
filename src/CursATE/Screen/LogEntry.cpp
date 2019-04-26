#include "CursATE/Screen/LogEntry.hpp"
#include "CursATE/Screen/detail/LogEntryDataSource.hpp"
#include "CursATE/Curses/Form.hpp"
#include "CursATE/Curses/Field/Button.hpp"
#include "CursATE/Curses/Field/Input.hpp"
#include "CursATE/Curses/Field/Radio.hpp"
#include "CursATE/Curses/ScrolableWindow.hpp"
#include "CursATE/Curses/ctrl.hpp"

using LogATE::Tree::FilterFactory;
using CursATE::Curses::Form;
using CursATE::Curses::KeyShortcuts;
using CursATE::Curses::Field::Button;
using CursATE::Curses::Field::Input;
using CursATE::Curses::Field::Radio;
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
  while(true)
  {
    win.refresh();
    switch( getch() )
    {
      case 10:
      case KEY_ENTER:
      case 'f':
           {
             auto ptr = createFilterBasedOnSelection( ds, *win.currentSelection() );
             if(ptr)
               return ptr;
             break;
           }
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


auto makeRegexInput(But::Optional<std::string> const& value)
{
  if(value)
    return Input{"regex", "^" + *value + "$"};  // TODO: add escaping of regex special values
  return Input{"regex"};
}


auto makeCompareRadio(But::Optional<std::string> const& value)
{
  const auto defaultSelection = value ? 1u : 0u;
  return Radio{"Compare", {"Key", "Value"}, defaultSelection};
}


std::shared_ptr<LogATE::Tree::Node> createGrep(detail::LogEntryDataSource const& ds, const Curses::DataSource::Id id, FilterFactory& ff)
{
  const auto value = ds.id2value(id);
  auto form = Form{ KeyShortcuts{
                                  {'n', "Name"},
                                  {'p', "Path"},
                                  {'r', "regex"},
                                  {'v', "Compare"},
                                  {'c', "Case"},
                                  {'s', "Search"},
                                  {'o', "ok"},
                                  {'q', "quit"}
                                },
                    Input{ "Name", "grep " + ds.id2path(id).str() },
                    Input{ "Path", ds.id2path(id).str() },
                    makeRegexInput(value),
                    makeCompareRadio(value),
                    Radio{ "Case", {"Sensitive", "Insensitive"} },
                    Radio{ "Search", {"Regular", "Inverse"} },
                    Button{"ok"},
                    Button{"quit"}
                  };
  const auto ret = form.process();
  if(ret[7] == "true")
    return {};
  BUT_ASSERT(ret[6] == "true" && "'OK' not clicked");
  FilterFactory::Options opts{
                               std::make_pair("Path",    ret[1]),
                               std::make_pair("regex",   ret[2]),
                               std::make_pair("Compare", ret[3]),
                               std::make_pair("Case",    ret[4]),
                               std::make_pair("Search",  ret[5]),
                             };
  auto ptr = ff.build( FilterFactory::Type{"Grep"}, FilterFactory::Name{ret[0]}, std::move(opts) );
  return std::move(ptr).underlyingPointer();
}


std::shared_ptr<LogATE::Tree::Node> createExplode(detail::LogEntryDataSource const& ds, const Curses::DataSource::Id id, FilterFactory& ff)
{
  const auto value = ds.id2value(id);
  auto form = Form{ KeyShortcuts{
                                  {'n', "Name"},
                                  {'p', "Path"},
                                  {'o', "ok"},
                                  {'q', "quit"}
                                },
                    Input{ "Name", "explode " + ds.id2path(id).str() },
                    Input{ "Path", ds.id2path(id).str() },
                    Button{"ok"},
                    Button{"quit"}
                  };
  const auto ret = form.process();
  if(ret[3] == "true")
    return {};
  BUT_ASSERT(ret[2] == "true" && "'OK' not clicked");
  FilterFactory::Options opts{ std::make_pair("Path", ret[1]) };
  auto ptr = ff.build( FilterFactory::Type{"Explode"}, FilterFactory::Name{ret[0]}, std::move(opts) );
  return std::move(ptr).underlyingPointer();
}
}


template<typename DS>
std::shared_ptr<LogATE::Tree::Node> LogEntry::createFilterBasedOnSelection(DS const& ds, const Curses::DataSource::Id id) const
{
  const auto filterName = selectFilter();
  if(not filterName)
    return {};
  const auto names = supportedFilters();
  if( *filterName == names[0] )
    return createGrep(ds, id, *filterFactory_);
  if( *filterName == names[1] )
    return createExplode(ds, id, *filterFactory_);
  /*
  if( *filterName == names[2] )
    return createAcceptAll(ds, id, *filterFactory_);
  */
  throw std::logic_error{"unsupported filter type: " + *filterName};
  (void)ds;
  (void)id;
}

}
