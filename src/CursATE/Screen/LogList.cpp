#include "CursATE/Screen/LogEntry.hpp"
#include "CursATE/Screen/LogList.hpp"
#include "CursATE/Screen/help.hpp"
#include "CursATE/Screen/FilterTree.hpp"
#include "CursATE/Screen/displayError.hpp"
#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/Curses/Form.hpp"
#include "CursATE/Curses/getChar.hpp"
#include "CursATE/Curses/ctrl.hpp"
#include "LogATE/Printers/OrderedPrettyPrint.hpp"
#include <But/assert.hpp>

using LogATE::Printers::OrderedPrettyPrint;
using LogATE::Tree::FilterFactory;
using CursATE::Curses::CursorVisibility;
using CursATE::Curses::CursorVisibilityGuard;
using CursATE::Curses::ctrl;
using CursATE::Curses::makeForm;
using CursATE::Curses::KeyShortcuts;
using CursATE::Curses::Field::Button;

namespace CursATE::Screen
{

namespace
{
auto makePrinter()
{
  // TODO: temporary hardcode
  OrderedPrettyPrint::SilentTags silent{{"But::PreciseDT", "Priority", "ComponentId", "UniqueId", "string"}};
  OrderedPrettyPrint::PriorityTags prio{{"But::PreciseDT", "Priority", "ComponentId", "UniqueId", "But::ThreadNo", "string"}};
  return OrderedPrettyPrint{ std::move(silent), std::move(prio) };
}

std::function<std::string()> threadsStats(LogATE::Utils::WorkerThreadsShPtr const& workers)
{
  auto wp = std::weak_ptr<LogATE::Utils::WorkerThreads>{ workers.underlyingPointer() };
  return [wp] {
    auto w = wp.lock();
    if(not w)
      return std::string{"T:0/0"};
    return "T:" + std::to_string( w->running() ) + "/" + std::to_string( w->threads() );
  };
}
}

LogList::LogList(LogATE::Utils::WorkerThreadsShPtr workers, std::function<size_t()> inputErrors):
  search_{workers},
  filterFactory_{workers},
  filterWindows_{ makePrinter(), std::move(inputErrors), threadsStats(workers) },
  root_{ filterFactory_.build( FilterFactory::Type{"AcceptAll"}, FilterFactory::Name{"all logs"}, FilterFactory::Options{} ) },
  currentNode_{root_},
  currentWindow_{ filterWindows_.window(currentNode_) }
{ }


void LogList::run()
{
  const CursorVisibilityGuard cvg(CursorVisibility::Invisible);
  do
  {
    currentWindow_->refresh();
    const auto ch = Curses::getChar( std::chrono::milliseconds{300} );
    if(ch)
      reactOnKey(*ch);
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
  FilterTree ft{root_};
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
  const auto sn = LogATE::SequenceNumber{id->value_};
  auto logs = currentNode_->logs().withLock()->from(sn, 1);
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
  const auto sn = LogATE::SequenceNumber{id->value_};
  centerAroundLog(node, sn);
}


void LogList::centerAroundLog(LogATE::Tree::NodeShPtr node, const LogATE::SequenceNumber sn)
{
  {
    auto win = filterWindows_.window(node);
    win->selectNearest( Curses::DataSource::Id{sn.value_} );
  }
  for(auto c: node->children())
    centerAroundLog(c, sn);
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
  const auto ret = search_.process( currentNode_, LogATE::SequenceNumber{selected->value_}, dir );
  if(not ret)
  {
    displayError({"no matching element found"});
    return;
  }
  currentWindow_->select( Curses::DataSource::Id{ret->value_} );
}


namespace
{
auto nextSn(LogATE::Tree::NodeShPtr node, const LogATE::SequenceNumber now)
{
  const auto ll = node->logs().withLock();
  const auto it = ll->find(now);
  if( it == ll->end() )
    return now;
  const auto next = it + 1;
  if( next == ll->end() )
    return now;
  return next->sequenceNumber();
}

auto prevSn(LogATE::Tree::NodeShPtr node, const LogATE::SequenceNumber now)
{
  const auto ll = node->logs().withLock();
  const auto it = ll->find(now);
  if( it == ll->end() )
    return now;
  if( it == ll->begin() )
    return it->sequenceNumber();
  const auto prev = it - 1;
  return prev->sequenceNumber();
}

auto moveSn(LogATE::Tree::NodeShPtr node, const LogATE::SequenceNumber now, const Search::Direction dir)
{
  switch(dir)
  {
    case Search::Direction::Forward:  return nextSn( std::move(node), now );
    case Search::Direction::Backward: return prevSn( std::move(node), now );
  }
  BUT_ASSERT(!"unkonw value of Search::Direction");
  throw std::logic_error{"unkonw value of Search::Direction when findinf adjisent iterator"};
}
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
  const auto start = moveSn( currentNode_, LogATE::SequenceNumber{selected->value_}, dir );
  const auto ret = search_.processAgain( currentNode_, LogATE::SequenceNumber{start.value_}, dir );
  if(not ret)
  {
    displayError({"no matching element found"});
    return;
  }
  currentWindow_->select( Curses::DataSource::Id{ret->value_} );
}

}
