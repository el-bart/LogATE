#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/Screen/LogEntry.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/TestHelpers.ut.hpp"

using namespace CursATE::Curses;
using LogATE::Tree::Node;
using LogATE::makeKey;

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
  return LogATE::Log{ makeKey(42), std::move(json) };
}
}


int main()
{
  const Init init;
  set(CursorVisibility::Invisible);

  const auto workers = But::makeSharedNN<LogATE::Utils::WorkerThreads>();
  LogATE::Tree::FilterFactory ff{workers};
  CursATE::Screen::LogEntry le{ ff, But::makeSharedNN<LogATE::Tree::Filter::AcceptAll>(workers, Node::Name{"fake root"}), makeLog() };
  const auto node = le.process();
  clear();
  refresh();
  if(node)
    mvprintw(0,0, "new node created!");
  else
    mvprintw(0,0, "no node created...");
}
