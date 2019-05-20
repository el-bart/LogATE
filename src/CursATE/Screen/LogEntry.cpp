#include "CursATE/Screen/LogEntry.hpp"
#include "CursATE/Screen/displayError.hpp"
#include "CursATE/Screen/help.hpp"
#include "CursATE/Screen/detail/LogEntryDataSource.hpp"
#include "CursATE/Screen/detail/formatAsPercentage.hpp"
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
using CursATE::Screen::displayError;

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

std::unique_ptr<LogATE::Tree::Node> LogEntry::process()
{
  const auto sp = ScreenPosition{ Row{1}, Column{1} };
  const auto ss = smallerScreenSize();
  const auto ds = But::makeSharedNN<detail::LogEntryDataSource>(log_);
  auto status = [ds](const size_t pos) { return detail::nOFmWithPercent(pos, ds->size()); };
  ScrolableWindow win{ ds, sp, ss, Window::Boxed::True, std::move(status) };
  return navigate(win, *ds);
}


template<typename Win, typename DS>
std::unique_ptr<LogATE::Tree::Node> LogEntry::navigate(Win& win, DS const& ds)
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

      case KEY_HOME: win.selectFirst(); break;
      case KEY_END:  win.selectLast(); break;
      case ctrl(KEY_HOME): win.scrollToLineBegin(); break;
      case ctrl(KEY_END):  win.scrollToLineEnd(); break;

      case 'H': win.selectFirstVisible(); break;
      case 'M': win.selectMiddleVisible(); break;
      case 'L': win.selectLastVisible(); break;

      case KEY_F(1): help(); break;

      // TODO: searching by string?
    }
  }
}


namespace
{
auto supportedFilters()
{
  return std::array<std::string, 6>{{"Grep", "Explode", "BinarySplit", "From", "To", "AcceptAll"}};
}


But::Optional<std::string> selectFilter()
{
  const auto names = supportedFilters();
  auto form = Form{ KeyShortcuts{
                                  {'g', names[0]},
                                  {'e', names[1]},
                                  {'b', names[2]},
                                  {'f', names[3]},
                                  {'t', names[4]},
                                  {'a', names[5]},
                                  {'c', "cancel"},
                                  {'q', "cancel"}
                                },
                    Button{names[0]},
                    Button{names[1]},
                    Button{names[2]},
                    Button{names[3]},
                    Button{names[4]},
                    Button{names[5]},
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


std::unique_ptr<LogATE::Tree::Node> createGrepCommon(detail::LogEntryDataSource const& ds,
                                                     const Curses::DataSource::Id id,
                                                     FilterFactory& ff,
                                                     std::string const& namePrefix,
                                                     std::string const& type)
{
  const auto value = ds.id2value(id);
  auto form = Form{ KeyShortcuts{
                                  {'n', "Name"},
                                  {'p', "Path"},
                                  {'r', "regex"},
                                  {'v', "Compare"},
                                  {'c', "Case"},
                                  {'s', "Search"},
                                  {'t', "Trim"},
                                  {'o', "ok"},
                                  {'q', "quit"}
                                },
                    Input{ "Name", namePrefix + " " + ds.id2path(id).str() },
                    Input{ "Path", ds.id2path(id).str() },
                    makeRegexInput(value),
                    makeCompareRadio(value),
                    Radio{ "Case", {"Sensitive", "Insensitive"} },
                    Radio{ "Search", {"Regular", "Inverse"} },
                    Radio{ "Trim", {"False", "True"} },
                    Button{"ok"},
                    Button{"quit"}
                  };
  while(true)
  {
    try
    {
      const auto ret = form.process();
      if(ret[8] == "true")
        return {};
      BUT_ASSERT(ret[7] == "true" && "'OK' not clicked");
      FilterFactory::Options opts{
          std::make_pair("Path",    ret[1]),
          std::make_pair("regex",   ret[2]),
          std::make_pair("Compare", ret[3]),
          std::make_pair("Case",    ret[4]),
          std::make_pair("Search",  ret[5]),
          std::make_pair("Trim",    ret[6])
      };
      auto ptr = ff.build( FilterFactory::Type{type}, FilterFactory::Name{ret[0]}, std::move(opts) );
      return std::move(ptr).underlyingPointer();
    }
    catch(std::exception const& ex)
    {
      displayError(ex);
    }
  }
}

std::unique_ptr<LogATE::Tree::Node> createGrep(detail::LogEntryDataSource const& ds, const Curses::DataSource::Id id, FilterFactory& ff)
{
  return createGrepCommon(ds, id, ff, "grep", "Grep");
}


std::unique_ptr<LogATE::Tree::Node> createBinarySplit(detail::LogEntryDataSource const& ds, const Curses::DataSource::Id id, FilterFactory& ff)
{
  return createGrepCommon(ds, id, ff, "split over", "BinarySplit");
}


std::unique_ptr<LogATE::Tree::Node> createExplode(detail::LogEntryDataSource const& ds,
                                                  const Curses::DataSource::Id id,
                                                  FilterFactory& ff)
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
  while(true)
  {
    try
    {
      const auto ret = form.process();
      if(ret[3] == "true")
        return {};
      BUT_ASSERT(ret[2] == "true" && "'OK' not clicked");
      FilterFactory::Options opts{ std::make_pair("Path", ret[1]) };
      auto ptr = ff.build( FilterFactory::Type{"Explode"}, FilterFactory::Name{ret[0]}, std::move(opts) );
      return std::move(ptr).underlyingPointer();
    }
    catch(std::exception const& ex)
    {
      displayError(ex);
    }
  }
}


std::unique_ptr<LogATE::Tree::Node> createFrom(FilterFactory& ff, const Curses::DataSource::Id id)
{
  auto form = Form{ KeyShortcuts{
                                  {'n', "Name"},
                                  {'e', "Edge"},
                                  {'s', "Edge"},
                                  {'o', "ok"},
                                  {'q', "quit"}
                                },
                    Input{ "Name", "from " + std::to_string(id.value_) },
                    Input{ "Edge", std::to_string(id.value_) },
                    Button{"ok"},
                    Button{"quit"}
                  };
  while(true)
  {
    try
    {
      const auto ret = form.process();
      if(ret[3] == "true")
        return {};
      BUT_ASSERT(ret[2] == "true" && "'OK' not clicked");
      FilterFactory::Options opts{ std::make_pair("Edge", ret[1]) };
      auto ptr = ff.build( FilterFactory::Type{"From"}, FilterFactory::Name{ret[0]}, std::move(opts) );
      return std::move(ptr).underlyingPointer();
    }
    catch(std::exception const& ex)
    {
      displayError(ex);
    }
  }
}


std::unique_ptr<LogATE::Tree::Node> createTo(FilterFactory& ff, const Curses::DataSource::Id id)
{
  auto form = Form{ KeyShortcuts{
                                  {'n', "Name"},
                                  {'e', "Edge"},
                                  {'s', "Edge"},
                                  {'o', "ok"},
                                  {'q', "quit"}
                                },
                    Input{ "Name", "to " + std::to_string(id.value_) },
                    Input{ "Edge", std::to_string(id.value_) },
                    Button{"ok"},
                    Button{"quit"}
                  };
  while(true)
  {
    try
    {
      const auto ret = form.process();
      if(ret[3] == "true")
        return {};
      BUT_ASSERT(ret[2] == "true" && "'OK' not clicked");
      FilterFactory::Options opts{ std::make_pair("Edge", ret[1]) };
      auto ptr = ff.build( FilterFactory::Type{"To"}, FilterFactory::Name{ret[0]}, std::move(opts) );
      return std::move(ptr).underlyingPointer();
    }
    catch(std::exception const& ex)
    {
      displayError(ex);
    }
  }
}


std::unique_ptr<LogATE::Tree::Node> createAcceptAll(FilterFactory& ff)
{
  auto form = Form{ KeyShortcuts{
                                  {'n', "Name"},
                                  {'t', "Trim"},
                                  {'o', "ok"},
                                  {'q', "quit"}
                                },
                    Input{ "Name", "accept all" },
                    Input{ "Trim", "" },
                    Button{"ok"},
                    Button{"quit"}
                  };
  while(true)
  {
    try
    {
      const auto ret = form.process();
      if(ret[3] == "true")
        return {};
      BUT_ASSERT(ret[2] == "true" && "'OK' not clicked");
      FilterFactory::Options opts{};
      if( not ret[1].empty() )
        opts["Trim"] = ret[1];
      auto ptr = ff.build( FilterFactory::Type{"AcceptAll"}, FilterFactory::Name{ret[0]}, std::move(opts) );
      return std::move(ptr).underlyingPointer();
    }
    catch(std::exception const& ex)
    {
      displayError(ex);
    }
  }
}
}


template<typename DS>
std::unique_ptr<LogATE::Tree::Node> LogEntry::createFilterBasedOnSelection(DS const& ds, const Curses::DataSource::Id id) const
{
  const auto filterName = selectFilter();
  if(not filterName) return {};
  const auto names = supportedFilters();
  if( *filterName == names[0] ) return createGrep(ds, id, *filterFactory_);
  if( *filterName == names[1] ) return createExplode(ds, id, *filterFactory_);
  if( *filterName == names[2] ) return createBinarySplit(ds, id, *filterFactory_);
  if( *filterName == names[3] ) return createFrom(*filterFactory_, Curses::DataSource::Id{log_.sequenceNumber().value_});
  if( *filterName == names[4] ) return createTo(*filterFactory_, Curses::DataSource::Id{log_.sequenceNumber().value_});
  if( *filterName == names[5] ) return createAcceptAll(*filterFactory_);
  throw std::logic_error{"unsupported filter type: " + *filterName};
}

}
