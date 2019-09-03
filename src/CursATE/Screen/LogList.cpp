#include "CursATE/Screen/LogEntry.hpp"
#include "CursATE/Screen/LogList.hpp"
#include "CursATE/Screen/help.hpp"
#include "CursATE/Screen/FilterTree.hpp"
#include "CursATE/Screen/displayError.hpp"
#include "CursATE/Screen/detail/id2key.hpp"
#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/Curses/Form.hpp"
#include "CursATE/Curses/Field/Button.hpp"
#include "CursATE/Curses/Field/Input.hpp"
#include "CursATE/Curses/Field/Radio.hpp"
#include "CursATE/Curses/getChar.hpp"
#include "CursATE/Curses/ctrl.hpp"
#include "LogATE/Printers/OrderedPrettyPrint.hpp"
#include "LogATE/Net/TcpClient.hpp"
#include "LogATE/Net/TcpRawClient.hpp"
#include <But/assert.hpp>
#include <But/Optional.hpp>
#include <boost/lexical_cast.hpp>
#include <cctype>
#include <tuple>

using LogATE::Printers::OrderedPrettyPrint;
using LogATE::Tree::FilterFactory;
using CursATE::Curses::CursorVisibility;
using CursATE::Curses::CursorVisibilityGuard;
using CursATE::Curses::ctrl;
using CursATE::Curses::makeForm;
using CursATE::Curses::KeyShortcuts;
using CursATE::Curses::Field::Button;
using CursATE::Curses::Field::Input;
using CursATE::Curses::Field::Radio;
using CursATE::Screen::detail::id2key;
using CursATE::Screen::detail::key2id;

namespace CursATE::Screen
{

namespace
{
std::function<std::string()> threadsStats(LogATE::Utils::WorkerThreadsShPtr const& workers)
{
  auto wp = std::weak_ptr<LogATE::Utils::WorkerThreads>{ workers.underlyingPointer() };
  return [wp] {
    auto w = wp.lock();
    if(not w)
      return std::string{"Q:0 T:0/0"};
    return "Q:" + std::to_string( w->queued() ) + " T:" + std::to_string( w->running() ) + "/" + std::to_string( w->threads() );
  };
}
}

LogList::LogList(LogATE::Utils::WorkerThreadsShPtr workers,
                 std::function<size_t()> inputErrors,
                 std::function<std::string(LogATE::Log const&)> log2str):
  workers_{ std::move(workers) },
  log2str_{ std::move(log2str) },
  search_{workers_},
  filterFactory_{workers_},
  filterWindows_{ log2str_, std::move(inputErrors), threadsStats(workers_) },
  root_{ filterFactory_.build( FilterFactory::Type{"AcceptAll"}, FilterFactory::Name{"all logs"}, FilterFactory::Options{} ) },
  currentNode_{root_},
  currentWindow_{ filterWindows_.window(currentNode_) }
{ }


void LogList::run()
{
  const CursorVisibilityGuard cvg(CursorVisibility::Invisible);
  do
  {
    try
    {
      currentWindow_->refresh();
      const auto ch = Curses::getChar( std::chrono::milliseconds{300} );
      if(ch)
        reactOnKey(*ch);
    }
    catch(std::exception const& ex)
    {
      displayError(ex);
    }
  }
  while(not quit_);
}


void LogList::reactOnKey(const int ch)
{
  switch(ch)
  {
    case 'q': processQuitProgram(); break;

    case KEY_UP:    currentWindow_->selectUp(); break;
    case KEY_DOWN:  currentWindow_->selectDown(); break;
    case KEY_LEFT:  currentWindow_->scrollLeft(); break;
    case KEY_RIGHT: currentWindow_->scrollRight(); break;

    case KEY_PPAGE: currentWindow_->selectPageUp(); break;
    case KEY_NPAGE: currentWindow_->selectPageDown(); break;

    case KEY_HOME: currentWindow_->selectFirst(); break;
    case KEY_END:  currentWindow_->selectLast(); break;
    case ctrl(KEY_HOME): currentWindow_->scrollToLineBegin(); break;
    case ctrl(KEY_END):  currentWindow_->scrollToLineEnd(); break;

    case 'H': currentWindow_->selectFirstVisible(); break;
    case 'M': currentWindow_->selectMiddleVisible(); break;
    case 'L': currentWindow_->selectLastVisible(); break;

    case 'j': centerAllChildrenAroundCurrentLog(); break;
    case 'J': centerAllNodesAroundCurrentLog(); break;

    case 'm': createMark(); break;
    case '`': gotoMarkLocal(); break;
    case '~': gotoMarkAbsolute(); break;

    case 't': processFilterTree(); break;
    case 10:
    case KEY_ENTER:
    case 'f': processLogEntry(); break;

    case '/': processSearch(Search::Direction::Forward); break;
    case 'n': processSearchAgain(Search::Direction::Forward); break;
    case '?': processSearch(Search::Direction::Backward); break;
    case 'N': processSearchAgain(Search::Direction::Backward); break;

    case 'h':
    case KEY_F(1): help(); break;

    case 'p': pipeLogsToHost(); break;

    // TODO: searching by regex?
    // TODO: moving to a log with a given ID?
  }
}


void LogList::processQuitProgram()
{
  currentWindow_->forceNextRefresh();
  auto form = makeForm( KeyShortcuts{
                          {'y', "exit"},
                          {'e', "exit"},
                          {'o', "exit"},
                          {'n', "cancel"},
                          {'q', "cancel"},
                          {'c', "cancel"}
                        },
                        Button{"cancel"},
                        Button{"exit"} );
  const auto ret = form.process();
  if( ret[1] == "false" )
    return;
  quit_ = true;
}


void LogList::processFilterTree()
{
  FilterTree ft{workers_, root_};
  currentNode_ = ft.selectNext(currentNode_);
  currentWindow_ = filterWindows_.window(currentNode_);
  currentWindow_->forceNextRefresh();
  filterWindows_.prune();
}


void LogList::processLogEntry()
{
  const auto id = currentWindow_->currentSelection();
  if(not id)
    return;
  const auto key = id2key(*id);
  auto logs = currentNode_->clogs()->withLock()->from(key, 1);
  if( logs.empty() )
    return;
  BUT_ASSERT( logs.size() == 1u );
  currentWindow_->forceNextRefresh();
  LogEntry le{filterFactory_, currentNode_, std::move(logs[0])};
  auto newNode = le.process();
  if(not newNode)
    return;
  try
  {
    currentNode_ = currentNode_->add( LogATE::Tree::NodePtr{ std::move(newNode) } );
    currentWindow_ = filterWindows_.window(currentNode_);
    currentWindow_->select(*id);
  }
  catch(std::exception const& ex)
  {
    displayError(ex);
  }
  currentWindow_->forceNextRefresh();
}


void LogList::centerAllChildrenAroundCurrentLog()
{
  centerAroundLogSelection(currentNode_);
}


void LogList::centerAllNodesAroundCurrentLog()
{
  centerAroundLogSelection(root_);
}


void LogList::centerAroundLogSelection(LogATE::Tree::NodeShPtr node)
{
  const auto id = currentWindow_->currentSelection();
  if(not id)
    return;
  const auto key = id2key(*id);
  centerAroundLog(node, key);
}


void LogList::centerAroundLog(LogATE::Tree::NodeShPtr node, LogATE::Log::Key const& key)
{
  {
    auto win = filterWindows_.window(node);
    win->selectNearest( key2id(key) );
  }
  for(auto c: node->children())
    centerAroundLog(c, key);
}


void LogList::processSearch(const Search::Direction dir)
{
  currentWindow_->forceNextRefresh();
  const auto selected = currentWindow_->currentSelection();
  if(not selected)
  {
    displayError({"window is empty"});
    return;
  }
  const auto ret = search_.process( currentNode_, id2key(*selected), dir );
  processSearchResult(ret);
}


void LogList::processSearchAgain(const Search::Direction dir)
{
  currentWindow_->forceNextRefresh();
  const auto selected = currentWindow_->currentSelection();
  if(not selected)
  {
    displayError({"window is empty"});
    return;
  }
  const auto ret = search_.processAgain( currentNode_, id2key(*selected), dir );
  processSearchResult(ret);
}


void LogList::processSearchResult(Search::Result const& result)
{
  switch(result.status_)
  {
    case Search::Result::Status::Canceled:
         return;
    case Search::Result::Status::NotFound:
         displayError({"no matching element found"});
         return;
    case Search::Result::Status::Found:
         BUT_ASSERT(result.key_);
         currentWindow_->select( key2id(*result.key_) );
         return;
  }
  BUT_ASSERT(!"code never reaches here");
  throw std::logic_error("code never reaches here");
}


namespace
{
But::Optional<std::tuple<std::string, LogATE::Net::Port, std::string>> hostPortDialog()
{
  auto form = makeForm( KeyShortcuts{
                          {'h', "Host"},
                          {'p', "Port"},
                          {'f', "Format"},
                          {'o', "ok"},
                          {ctrl('o'), "ok"},
                          {'q', "quit"},
                          {'e', "quit"},
                          {ctrl('e'), "quit"}
                        },
                        Input{"Host", "127.0.0.1"},
                        Input{"Port", "6666"},
                        Radio{"Format", {"text", "json"}},
                        Button{"ok"},
                        Button{"quit"} );
  const auto ret = form.process();
  if( ret[3] == "false" )
    return {};
  auto host = std::string{ret[0]};
  auto port = LogATE::Net::Port{ boost::lexical_cast<uint16_t>(ret[1]) };
  auto format = std::string{ret[2]};
  return std::make_tuple( std::move(host), port, std::move(format) );
}


auto copyLogs(LogATE::Tree::NodeWeakPtr weakNode)
{
  std::deque<LogATE::Log> logs;
  auto node = weakNode.lock();
  if(not node)
    return logs;
  const auto ll = node->clogs()->withLock();
  for(auto it=ll->begin(); it!=ll->end(); ++it)
    logs.push_back(*it);
  return logs;
}


template<typename Formatter>
void streamLogs(LogATE::Tree::NodeWeakPtr weakNode, std::string const& host, const LogATE::Net::Port port, Formatter formatter)
{
  LogATE::Net::TcpRawClient sink{host, port};
  const auto logs = copyLogs(weakNode);
  for(auto& log: logs)
  {
    sink.write( formatter(log) );
    sink.write("\n");
  }
}
}


void LogList::pipeLogsToHost()
{
  currentWindow_->forceNextRefresh();
  const auto hp = hostPortDialog();
  if(not hp)
    return;
  auto wp = LogATE::Tree::NodeWeakPtr{ currentNode_.underlyingPointer() };

  if( std::get<2>(*hp) == "json" )
  {
    streamLogs( wp, std::get<0>(*hp), std::get<1>(*hp), [](auto& log) { return log.str(); } );
    return;
  }
  if( std::get<2>(*hp) == "text" )
  {
    streamLogs( wp, std::get<0>(*hp), std::get<1>(*hp), log2str_ );
    return;
  }
  BUT_ASSERT(!"unknown sink found - update the code...");
  throw std::logic_error{"unknown sink found - update the code..."};
}


namespace
{
auto getMarkKey()
{
  const auto ch = Curses::getChar();
  if( not isprint(ch) )
    BUT_THROW(LogList::InvalidMark, "mark must be a printable character, instead got code=" << int{ch});
  return ch;
}

auto currentLog(Curses::ScrolableWindow const& win, LogATE::Tree::NodeShPtr const& node)
{
  const auto sel = win.currentSelection();
  if(not sel)
    BUT_THROW(LogList::InvalidMark, "empty view - cannot create a mark");
  const auto key = id2key(*sel);
  const auto& ll = node->clogs()->withLock();
  const auto it = ll->find(key);
  if( it == ll->end() )
    BUT_THROW(LogList::InvalidMark, "log no longer available (background prune?): " << key2id(key).value_);
  return *it;
}
}


void LogList::createMark()
{
  const auto log = currentLog(*currentWindow_, currentNode_);
  const auto ch = getMarkKey();
  marks_.insert( ch, detail::Marks::Entry{log.key(), currentNode_} );
  marks_.prune();
}


namespace
{
auto getMarkEntry(detail::Marks const& marks, const char ch)
{
  auto e = marks.find(ch);
  if(not e)
    BUT_THROW(LogList::InvalidMark, "no mark registered for character '" << char(ch) << "' (code=" << ch << ")");
  return std::move(*e);
}
}


void LogList::gotoMarkLocal()
{
  const auto ch = getMarkKey();
  const auto e = getMarkEntry(marks_, ch);
  const auto id = key2id(e.key_);
  currentWindow_->selectNearest(id);
}


void LogList::gotoMarkAbsolute()
{
  const auto ch = getMarkKey();
  const auto e = getMarkEntry(marks_, ch);
  const auto nodeSh = e.node_.lock();
  if(not nodeSh)
    BUT_THROW(LogList::InvalidMark, "node for mark '" << char(ch) << "' (code=" << ch << ") no longer exists");
  const auto node = LogATE::Tree::NodeShPtr{nodeSh};
  auto win = filterWindows_.window(node);
  win->selectNearest( key2id(e.key_) );
  currentWindow_ = win;
  currentNode_ = node;
}

}
