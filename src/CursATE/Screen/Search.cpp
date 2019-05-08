#include "CursATE/Screen/Search.hpp"
#include "CursATE/Screen/ProgressBar.hpp"
#include "CursATE/Curses/Form.hpp"
#include "LogATE/Utils/matchesLog.hpp"

using LogATE::Log;
using LogATE::SequenceNumber;
using LogATE::Utils::matchesAnyKey;
using LogATE::Utils::matchesAnyValue;


namespace CursATE::Screen
{

But::Optional<LogATE::SequenceNumber> Search::process(LogATE::Tree::NodeShPtr node, const LogATE::SequenceNumber currentSelection)
{
  if( not updateSearchPattern() )
    return {};
  return triggerSearch(node, currentSelection);
}


But::Optional<LogATE::SequenceNumber> Search::processAgain(LogATE::Tree::NodeShPtr node, const LogATE::SequenceNumber currentSelection)
{
  if( keyQuery_.empty() && valueQuery_.empty() )
    return {};
  return triggerSearch(node, currentSelection);
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
  But::Optional<LogATE::SequenceNumber> operator()() const
  {
    BUT_ASSERT( not key_.empty() || not value_.empty() );
    BUT_ASSERT( logs_.size() == monitor_->totalSize_ );
    for(auto& log: logs_)
    {
      if(monitor_->abort_)
        return {};
      const auto mk = (not key_.empty())   ? matchesAnyKey(log, key_)     : true;
      const auto mv = (not value_.empty()) ? matchesAnyValue(log, value_) : true;
      if( mk && mv )
      {
        monitor_->done_ = true;
        return log.sequenceNumber();
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


auto extractLogs(LogATE::Tree::NodeShPtr node, const LogATE::SequenceNumber currentSelection)
{
  const auto ll = node->logs().withLock();
  const auto it = ll->find(currentSelection);
  return std::vector<Log>{it, ll->end()};
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


But::Optional<LogATE::SequenceNumber> Search::triggerSearch(LogATE::Tree::NodeShPtr node, const LogATE::SequenceNumber currentSelection)
{
  auto logs = extractLogs(node, currentSelection);
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
