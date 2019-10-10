#include "CursATE/Screen/Search.hpp"
#include "CursATE/Screen/ProgressBar.hpp"
#include "CursATE/Screen/help.hpp"
#include "CursATE/Curses/Form.hpp"
#include "CursATE/Curses/ctrl.hpp"
#include "LogATE/Utils/matchesLog.hpp"
#include "LogATE/Tree/Search.hpp"

using LogATE::Log;
using LogATE::Utils::matchesAnyKey;
using LogATE::Utils::matchesAnyValue;
using LogATE::Utils::matchesAnyKeyValue;
using CursATE::Curses::ctrl;
using TreeSearch = LogATE::Tree::Search;


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
std::function<bool(LogATE::Log const&)> buildQuery(std::string const& key, std::string const& value)
{
  const auto mode = (key.empty()?0x00:0x10) | (value.empty()?0x00:0x1);
  switch(mode)
  {
    case 0x10: return [key]       (LogATE::Log const& log) { return matchesAnyKey(log, key); };
    case 0x01: return [value]     (LogATE::Log const& log) { return matchesAnyValue(log, value); };
    case 0x11: return [key, value](LogATE::Log const& log) { return matchesAnyKeyValue(log, key, value); };
  }
  BUT_ASSERT(!"unknown mode");
  throw std::logic_error{"unknown mode"};
}


bool hasResultEarly(ProgressBar::Monitor const& monitor)
{
  using Clock = std::chrono::steady_clock;
  const auto timeout = std::chrono::milliseconds{300};
  const auto deadline = Clock::now() + timeout;
  while( Clock::now() < deadline )
    if( monitor.done() )
      return true;
    else
      std::this_thread::yield();
  return false;
}


struct SearchProgressMonitor: ProgressBar::Monitor
{
  explicit SearchProgressMonitor(LogATE::Tree::Search::Result& result): result_{&result} { }

  uint64_t totalSize() const override
  {
    BUT_ASSERT(result_);
    return result_->requiredCompares_->load();
  }

  uint64_t processed() const override
  {
    BUT_ASSERT(result_);
    return result_->comparesDone_->load();
  }

  bool done() const override
  {
    BUT_ASSERT(result_);
    return result_->value_.wait_for( std::chrono::seconds{0} ) == std::future_status::ready;
  }

  bool aborted() const override
  {
    BUT_ASSERT(result_);
    return result_->cancel_->load();
  }

  void abort() override
  {
    BUT_ASSERT(result_);
    *result_->cancel_ = true;
  }

private:
  LogATE::Tree::Search::Result* result_{nullptr};
};


auto dir2dir(const Search::Direction dir)
{
  switch(dir)
  {
    case Search::Direction::Forward:  return LogATE::Tree::Search::Direction::Forward;
    case Search::Direction::Backward: return LogATE::Tree::Search::Direction::Backward;
  }
  BUT_ASSERT(!"missing case in Direcion enum");
  throw std::logic_error{"missing case in Direcion enum"};
}


auto translate(LogATE::Tree::Search::Result&& res)
{
  BUT_ASSERT( res.value_.wait_for( std::chrono::seconds{0} ) == std::future_status::ready );
  auto opt = res.value_.get();
  if(opt)
    return Search::Result::found( std::move(*opt) );
  if(res.cancel_)
    return Search::Result::canceled();
  return Search::Result::notFound();
}


But::Optional<LogATE::Log::Key> advanceByOne(LogATE::Tree::Logs const& logs,
                                             LogATE::Log::Key const& currentSelection,
                                             const Search::Direction dir)
{
  switch(dir)
  {
    case Search::Direction::Forward:
         {
           const auto data = logs.withLock()->from(currentSelection, 2);
           if( data.size() < 2u )
             return {};
           return data.back().key();
         }
    case Search::Direction::Backward:
         {
           const auto data = logs.withLock()->to(currentSelection, 2);
           if( data.size() < 2u )
             return {};
           return data.front().key();
         }
  }
  BUT_ASSERT(!"unknown search direction");
  throw std::logic_error{"unknown search direction"};
}
}


Search::Result Search::triggerSearch(LogATE::Tree::NodeShPtr node,
                                     LogATE::Log::Key const& currentSelection,
                                     const Direction dir)
{
  LogATE::Tree::Search engine{workers_};
  auto query = buildQuery(keyQuery_, valueQuery_);
  auto startPoint = advanceByOne( *node->clogs(), currentSelection, dir );
  if(not startPoint)
    return Result::notFound();
  auto result = engine.search( node->clogs(), std::move(*startPoint), dir2dir(dir), std::move(query) );
  const auto monitor = But::makeSharedNN<SearchProgressMonitor>(result);
  if( hasResultEarly(*monitor) )
    return translate( std::move(result) );
  ProgressBar pb{monitor};
  if( not pb.process() )
    return Result::canceled();
  return translate( std::move(result) );
}

}
