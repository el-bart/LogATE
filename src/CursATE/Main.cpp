#include "CursATE/Main.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"

#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/Screen/detail/LogDataSource.hpp"
#include <sstream>

using namespace CursATE::Curses;          

namespace CursATE
{

Main::Main(const LogATE::Net::Port port):
  root_{ logList_.root() },
  server_{port},
  dataPump_{ [&] { this->dataPumpLoop(); } }
{ }

Main::~Main()
{
  stop();
}

void Main::stop()
{
  quit_ = true;
  server_.interrupt();
  logList_.stop();
  clear();
  refresh();
}

void Main::run()
{
  logList_.run();
  stop();
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
      // LOG DOWN! MEDIC!
    }
}

}
