#include "CursATE/Screen/Search.hpp"
#include "CursATE/Screen/ProgressBar.hpp"
#include "CursATE/Screen/help.hpp"
#include "CursATE/Curses/Form.hpp"
#include "CursATE/Curses/ctrl.hpp"
#include "LogATE/Utils/matchesLog.hpp"

using LogATE::Log;
using LogATE::Utils::matchesAnyKey;
using LogATE::Utils::matchesAnyValue;
using LogATE::Utils::matchesAnyKeyValue;
using CursATE::Curses::ctrl;


namespace CursATE::Screen
{

Search::Result Search::process(LogATE::Tree::NodeShPtr node,
                               LogATE::Log::Key const& currentSelection,
                               const Direction dir)
{
  if( not updateSearchPattern() )
    return Result::canceled();
  return triggerSearch(node, currentSelection, dir);
}


Search::Result Search::processAgain(LogATE::Tree::NodeShPtr node,
                                    LogATE::Log::Key const& currentSelection,
                                    const Direction dir)
{
  if( keyQuery_.empty() && valueQuery_.empty() )
    return Result::canceled();
  return triggerSearch(node, currentSelection, dir);
}


bool Search::updateSearchPattern()
{
  auto form = Curses::makeForm(
        Curses::KeyShortcuts{
          {'s', "search"},
          {'o', "search"},
          {ctrl('o'), "search"},
          {'c', "cancel"},
          {'q', "cancel"},
          {'e', "cancel"},
          {ctrl('e'), "cancel"}
        },
        Curses::Field::Input{"key", keyQuery_},
        Curses::Field::Input{"value", valueQuery_},
        Curses::Field::Button{"search"},
        Curses::Field::Button{"cancel"}
      );
  auto ret = form.process();

  if( ret[2] != "true" )            // cancel
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
template<typename It>
auto nextIt(It it) { return ++it; }


template<typename Out, typename It>
auto copyN(It begin, It end, const size_t chunk)
{
  Out out;
  out.reserve(chunk);
  auto done = size_t{0};
  for(auto it=begin; it!=end && done!=chunk; ++it, ++done)
    out.push_back(*it);
  return out;
}


auto extractLogs(LogATE::Tree::NodeShPtr node, LogATE::Log::Key const& currentSelection, const Search::Direction dir, const size_t chunk)
{
  using Out = std::vector<Log>;
  const auto ll = node->clogs()->withLock();
  const auto it = ll->find(currentSelection);
  switch(dir)
  {
    case Search::Direction::Forward:
         if( it == ll->end() )
           return Out{};
         return copyN<Out>( nextIt(it), ll->end(), chunk );
    case Search::Direction::Backward:
         if( it == ll->begin() )
           return Out{};
         return copyN<Out>( LogATE::Tree::Logs::const_reverse_iterator{it}, ll->rend(), chunk );
  }
  BUT_ASSERT(!"invalid search direction");
  throw std::logic_error{"unknown value for Search::Direction"};
}


struct SearchQuery
{
  Search::Result operator()() const
  {
    BUT_ASSERT( not key_.empty() || not value_.empty() );
    const auto mode = (key_.empty()?0x00:0x10) | (value_.empty()?0x00:0x1);
    const auto chunkSize = 10'000u;
    auto startPoint = initialStartingPoint_;

    while(true)
    {
      const auto nodeTmp = node_.lock();
      if(not nodeTmp)
        break;
      const auto node = LogATE::Tree::NodeShPtr{nodeTmp};
      updateProgress(node, dir_);
      const auto logs = extractLogs(node, startPoint, dir_, chunkSize);
      const auto ret = searchInLogsChunk(logs, mode);
      if(ret)
        return *ret;
      if( logs.size() < chunkSize )
        break;
      startPoint = logs.back().key();
    }

    monitor_->done_ = true;
    return Search::Result::notFound();
  }

  void updateProgress(LogATE::Tree::NodeShPtr const& node, const Search::Direction dir) const
  {
    const auto ll = node->clogs()->withLock();
    const auto size = ll->size();
    const auto startIndex = ll->index(initialStartingPoint_);
    BUT_ASSERT( startIndex < size );
    switch(dir)
    {
      case Search::Direction::Forward:
           monitor_->totalSize_ = size - startIndex;
           return;
      case Search::Direction::Backward:
           monitor_->totalSize_ = startIndex;
           return;
    }
    BUT_ASSERT(!"code never reaches here");
  }

  But::Optional<Search::Result> searchInLogsChunk(std::vector<Log> const& logs, const int mode) const
  {
    for(auto& log: logs)
    {
      if(monitor_->abort_)
        return Search::Result::canceled();

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
        return Search::Result::found( log.key() );
      }
      ++monitor_->processed_;
    }
    return {};
  }

  std::string key_;
  std::string value_;
  LogATE::Log::Key initialStartingPoint_;
  LogATE::Tree::NodeWeakPtr node_;
  Search::Direction dir_;
  ProgressBar::MonitorShPtr monitor_;
};


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


Search::Result Search::triggerSearch(LogATE::Tree::NodeShPtr node,
                                     LogATE::Log::Key const& currentSelection,
                                     const Direction dir)
{
  const auto monitor = But::makeSharedNN<ProgressBar::Monitor>( node->clogs()->withLock()->size() );
  auto query = SearchQuery{keyQuery_, valueQuery_, currentSelection, node.underlyingPointer(), dir, monitor};
  auto ret = workers_->enqueueBatch( [q=std::move(query)] { return q(); } );
  if( hasResultEarly(*monitor) )
    return ret.get();
  ProgressBar pb{monitor};
  if( not pb.process() )
    return Result::canceled();
  return ret.get();
}

}
