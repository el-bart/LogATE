#include "CursATE/Main.hpp"

namespace CursATE
{

namespace
{
auto threadsCount()
{
  const auto possible = std::max( std::thread::hardware_concurrency() / 2u, 1u );
  const auto preferred = std::min(possible, 2u);
  return preferred;
}

}

Main::Main(const LogATE::Net::Port port):
  server_{workers_, port, LogATE::Net::TcpServer::JsonParsingMode::ParseToEndOfJson},
  logList_{ workers_, [&] { return server_.errors(); } },
  root_{ logList_.root() }
{
  const auto threads = threadsCount();
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
