#include "CursATE/Screen/Search.hpp"
#include "CursATE/Screen/ProgressBar.hpp"
#include "CursATE/Screen/help.hpp"
#include "CursATE/Curses/Form.hpp"
#include "LogATE/Utils/matchesLog.hpp"

using LogATE::Log;
using LogATE::Utils::matchesAnyKey;
using LogATE::Utils::matchesAnyValue;
using LogATE::Utils::matchesAnyKeyValue;


namespace CursATE::Screen
{

But::Optional<LogATE::Log::Key> Search::process(LogATE::Tree::NodeShPtr node,
                                                      LogATE::Log::Key const& currentSelection,
                                                      const Direction dir)
{
  if( not updateSearchPattern() )
    return {};
  return triggerSearch(node, currentSelection, dir);
}


But::Optional<LogATE::Log::Key> Search::processAgain(LogATE::Tree::NodeShPtr node,
                                                           LogATE::Log::Key const& currentSelection,
                                                           const Direction dir)
{
  if( keyQuery_.empty() && valueQuery_.empty() )
    return {};
  return triggerSearch(node, currentSelection, dir);
}


bool Search::updateSearchPattern()
{
  auto form = Curses::makeForm(
        Curses::KeyShortcuts{
          {'o', "search"},
          {'s', "search"},
          {'c', "cancel"},
          {'q', "cancel"}
        },
        Curses::Field::Input{"key", keyQuery_},
        Curses::Field::Input{"value", valueQuery_},
        Curses::Field::Button{"search"},
        Curses::Field::Button{"cancel"}
      );
  auto ret = form.process();

  if( ret[3] == "true" )            // cancel
    return false;

  BUT_ASSERT( ret[2] == "true" );   // search
  keyQuery_ = std::move(ret[0]);
  valueQuery_ = std::move(ret[1]);

  if( keyQuery_.empty() && valueQuery_.empty() )
    return false;
  return true;
}


namespace
{
struct SearchQuery
{
  But::Optional<LogATE::Log::Key> operator()() const
  {
    BUT_ASSERT( not key_.empty() || not value_.empty() );
    BUT_ASSERT( logs_.size() == monitor_->totalSize_ );
    const auto mode = (key_.empty()?0x00:0x10) | (value_.empty()?0x00:0x1);
    for(auto& log: logs_)
    {
      if(monitor_->abort_)
        return {};

      auto found = false;
      switch(mode)
      {
        case 0x10: found = matchesAnyKey(log, key_); break;
        case 0x01: found = matchesAnyValue(log, value_); break;
        case 0x11: found = matchesAnyKeyValue(log, key_, value_); break;
      }
      if(found)
      {
        monitor_->done_ = true;
        return log.key();
      }
      ++monitor_->processed_;
    }
    monitor_->done_ = true;
    return {};
  }

  std::string key_;
  std::string value_;
  std::vector<Log> logs_;
  ProgressBar::MonitorShPtr monitor_;
};


auto extractLogs(LogATE::Tree::NodeShPtr node, LogATE::Log::Key const& currentSelection, const Search::Direction dir)
{
  using Out = std::vector<Log>;
  const auto ll = node->logs().withLock();
  const auto it = ll->find(currentSelection);
  switch(dir)
  {
    case Search::Direction::Forward:  return Out{it, ll->end()};
    case Search::Direction::Backward: return Out{ std::set<Log>::const_reverse_iterator{it}, ll->rend() };
  }
  BUT_ASSERT(!"invalid search direction");
  throw std::logic_error{"unknown value for Search::Direction"};
}


bool hasResultEarly(ProgressBar::Monitor const& monitor)
{
  using Clock = std::chrono::steady_clock;
  const auto timeout = std::chrono::milliseconds{300};
  const auto deadline = Clock::now() + timeout;
  while( Clock::now() < deadline )
    if(monitor.done_)
      return true;
    else
      std::this_thread::yield();
  return false;
}
}


But::Optional<LogATE::Log::Key> Search::triggerSearch(LogATE::Tree::NodeShPtr node,
                                                      LogATE::Log::Key const& currentSelection,
                                                      const Direction dir)
{
  auto logs = extractLogs(node, currentSelection, dir);
  const auto monitor = But::makeSharedNN<ProgressBar::Monitor>( logs.size() );
  auto query = SearchQuery{keyQuery_, valueQuery_, std::move(logs), monitor};
  auto ret = workers_->enqueue( [q=std::move(query)] { return q(); } );
  if( hasResultEarly(*monitor) )
    return ret.get();
  ProgressBar pb{monitor};
  if( not pb.process() )
    return {};
  return ret.get();
}

}
