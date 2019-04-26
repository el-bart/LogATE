#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/Screen/LogEntry.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"

using namespace CursATE::Curses;
using LogATE::Tree::Node;

namespace
{
auto makeLog()
{
  nlohmann::json json;
  json["foo"] = "bar";
  json["answer"] = 42;
  json["basic types"]["string"] = "some text";
  json["basic types"]["int"] = 42;
  json["basic types"]["float"] = 4.2;
  json["basic types"]["bool"] = true;
  return LogATE::Log{ LogATE::SequenceNumber{42}, But::makeSharedNN<nlohmann::json>( std::move(json) ) };
}
}


int main()
{
  const Init init;
  set(CursorVisibility::Invisible);

  CursATE::Screen::LogEntry le{ But::makeSharedNN<LogATE::Tree::Filter::AcceptAll>(Node::Name{"fake root"}), makeLog() };
  const auto node = le.process();
  clear();
  refresh();
  if(node)
    mvprintw(0,0, "new node created!");
  else
    mvprintw(0,0, "no node created...");
}