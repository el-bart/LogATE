#include "CursATE/Main.hpp"
#include "LogATE/Printers/OrderedPrettyPrint.hpp"

namespace CursATE
{

namespace
{
auto threadsCount()
{
  const auto threads = std::max( std::thread::hardware_concurrency(), 1u );
  const auto preferred = std::max(threads/2u, 1u);
  return preferred;
}

auto makePrinter(Config const& config)
{
  return LogATE::Printers::OrderedPrettyPrint{config.silentTags_, config.priorityTags_};
}
}

Main::Main(Config const& config):
  server_{workers_, config.port_, config.keyExtractor_, config.jsonParsingMode_},
  logList_{ workers_, [&] { return server_.errors() + errors_; }, makePrinter(config), config.trimFields_ },
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
      auto logs = server_.readNextLogs();
      for(auto&& log: std::move(logs))
        if( log.json().is_object() ) // TODO: for now top level element must be an object
          root_->insert( std::move(log) );
        else
          ++errors_;
    }
    catch(...)
    {
      ++errors_;
      // LOG DOWN! MEDIC!
    }
}

}
