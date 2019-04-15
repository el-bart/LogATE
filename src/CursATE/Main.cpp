#include "CursATE/Main.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"

#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/LogDataSource.hpp"
#include <sstream>

using namespace CursATE::Curses;          

namespace CursATE
{

namespace
{
std::string log2str(LogATE::Log const& in)
{
  std::stringstream ss;
  ss << *in.log_;
  return ss.str();
}
}

Main::Main(const LogATE::Net::Port port):
  root_{ But::makeSharedNN<LogATE::Tree::Filter::AcceptAll>(LogATE::Tree::Node::Name{"root"}) },
  server_{port},
  dataPump_{ [&] { this->dataPumpLoop(); } },
  win_{ But::makeSharedNN<LogDataSource>(root_, log2str), ScreenPosition{Row{0}, Column{0}}, ScreenSize{stdscr}, Window::Boxed::True }
{
}

Main::~Main()
{
  stop();
}

void Main::stop()
{
  quit_ = true;
  server_.interrupt();
}

void Main::run()
{
  set(CursorVisibility::Invisible);
  do
  {
    win_.refresh();
  }
  while( getch() != 'q' );
  stop();
  // TODO
}

void Main::dataPumpLoop()
{
  while(not quit_)
    try
    {
      auto opt = server_.readNextLog();
      if(not opt)
        continue;
      root_->insert( std::move(*opt) );
    }
    catch(...)
    {
      // log down! MEDIC!
    }
}

}
