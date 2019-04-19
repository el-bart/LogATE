#include "CursATE/Screen/LogList.hpp"
#include "CursATE/Screen/LogDisplay/jsonLine.hpp"
#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/Curses/ctrl.hpp"

using LogATE::Tree::FilterFactory;
using CursATE::Curses::CursorVisibility;
using CursATE::Curses::CursorVisibilityGuard;
using CursATE::Curses::ctrl;

namespace CursATE::Screen
{

LogList::LogList():
  filterWindows_{ LogDisplay::jsonLine },
  root_{ filterFactory_.build( FilterFactory::Type{"AcceptAll"}, FilterFactory::Name{"all logs"}, FilterFactory::Options{} ) },
  currentWindow_{ filterWindows_.window(root_) }
{ }


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
    case 'q': quit_ = true; break;

    case KEY_UP:    currentWindow_->selectUp(); break;
    case KEY_DOWN:  currentWindow_->selectDown(); break;
    case KEY_LEFT:  currentWindow_->scrollLeft(); break;
    case KEY_RIGHT: currentWindow_->scrollRight(); break;

    case KEY_PPAGE: currentWindow_->selectPageUp(); break;
    case KEY_NPAGE: currentWindow_->selectPageDown(); break;

    case ctrl(KEY_HOME): currentWindow_->selectFirst(); break;
    case ctrl(KEY_END):  currentWindow_->selectLast(); break;
    case KEY_HOME: currentWindow_->scrollToLineBegin(); break;
    case KEY_END:  currentWindow_->scrollToLineEnd(); break;

    // TODO: enter for previewing log entry
    // TODO: 't' for opening log tree
    // TODO: searching by string?
    // TODO: moving to a log with a given ID?
  }
}

}
