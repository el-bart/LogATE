#include "CursATE/Screen/LogEntry.hpp"
#include "CursATE/Screen/LogList.hpp"
#include "CursATE/Screen/FilterTree.hpp"
#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/Curses/Form.hpp"
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
  OrderedPrettyPrint::SilentTags silent{{"env"}};
  OrderedPrettyPrint::PriorityTags prio{{"date", "LOG_NUMBER", "male_name", "misc"}};
  return OrderedPrettyPrint{ std::move(silent), std::move(prio) };
}
}

LogList::LogList(LogATE::Utils::WorkerThreadsShPtr workers):
  search_{workers},
  filterFactory_{ std::move(workers) },
  filterWindows_{ makePrinter() },
  root_{ filterFactory_.build( FilterFactory::Type{"AcceptAll"}, FilterFactory::Name{"all logs"}, FilterFactory::Options{} ) },
  currentNode_{root_},
  currentWindow_{ filterWindows_.window(currentNode_) }
{
  // TODO: temporary filte tree - just for fun....
  auto a = filterFactory_.build( FilterFactory::Type{"AcceptAll"},
                                 FilterFactory::Name{"copy of parent"},
                                 FilterFactory::Options{} );
  auto b = filterFactory_.build( FilterFactory::Type{"Grep"},
                                 FilterFactory::Name{"select even logs"},
                                 FilterFactory::Options{
                                                         {"Path", ".LOG_NUMBER"},
                                                         {"regex", "[02468]$"},
                                                         {"Compare", "Value"},
                                                         {"Case", "Sensitive"},
                                                         {"Search", "Regular"}
                                                       } );
  auto c = filterFactory_.build( FilterFactory::Type{"Grep"},
                                 FilterFactory::Name{"select odd timestamp"},
                                 FilterFactory::Options{
                                                         {"Path", ".date"},
                                                         {"regex", "[13579]\\."},
                                                         {"Compare", "Value"},
                                                         {"Case", "Sensitive"},
                                                         {"Search", "Regular"}
                                                       } );
  auto d = filterFactory_.build( FilterFactory::Type{"Explode"},
                                 FilterFactory::Name{"split by name"},
                                 FilterFactory::Options{ {"Path", ".male_name"} } );
  b->add( std::move(a) );
  root_->add( std::move(b) );
  root_->add( std::move(c) );
  root_->add( std::move(d) );
}


void LogList::run()
{
  const CursorVisibilityGuard cvg(CursorVisibility::Invisible);
  do
  {
    currentWindow_->refresh();
    const auto ch = getChar();
    if(ch)
      reactOnKey(*ch);
  }
  while(not quit_);
}


But::Optional<int> LogList::getChar() const
{
  timeout(300);
  const auto ch = getch();
  timeout(-1);
  if(ch == ERR)
    return {};
  return ch;
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

    case 'j': centerAllChildrenAroundCurrentLog(); break;
    case 'J': centerAllNodesAroundCurrentLog(); break;

    case 't': processFilterTree(); break;
    case 10:
    case KEY_ENTER:
    case 'f': processLogEntry(); break;

    case '/': processSearch(); break;
    // TODO: repeat last search
    // TODO: search backward

    // TODO: searching by string?
    // TODO: moving to a log with a given ID?
    // TODO: move selection to screen begin/center/end
  }
}


void LogList::processQuitProgram()
{
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
  LogEntry le{filterFactory_, currentNode_, std::move(logs[0])};
  auto newNode = le.process();
  if(not newNode)
    return;
  currentNode_ = currentNode_->add( LogATE::Tree::NodePtr{ std::move(newNode) } );
  currentWindow_ = filterWindows_.window(currentNode_);
  currentWindow_->select(*id);
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


void LogList::processSearch()
{
  const auto selected = currentWindow_->currentSelection();
  if(not selected)
    return;
  const auto ret = search_.process( currentNode_, LogATE::SequenceNumber{selected->value_} );
  (void)ret;
  // TODO
}

}
