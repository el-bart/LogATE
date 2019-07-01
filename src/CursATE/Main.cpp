#include "CursATE/Main.hpp"

namespace CursATE
{

Main::Main(const LogATE::Net::Port port):
  server_{workers_, port},
  logList_{ workers_, [&] { return server_.errors(); } },
  root_{ logList_.root() }
{
  const auto threads = std::max( std::thread::hardware_concurrency() / 2, 1u );
  for(auto i=0u; i<threads; ++i)
    dataPumpThreads_.emplace_back( [&] { this->dataPumpLoop(); } );
}

Main::~Main()
{
  stop();
  clear();
  mvwprintw(stdscr, 0,0, "waiting for server to shut down...");
  refresh();
}

void Main::stop()
{
  quit_ = true;
  for(auto i=0u; i<dataPumpThreads_.size(); ++i)
    server_.interrupt();
  logList_.stop();
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
