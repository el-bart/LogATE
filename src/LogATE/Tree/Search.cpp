#include "LogATE/Tree/Search.hpp"
#include "But/Threading/LockProxyProvider.hpp"
#include "But/Threading/BasicLockable.hpp"
#include "But/Guard.hpp"
#include "But/assert.hpp"

namespace LogATE::Tree
{

Search::Search(Utils::WorkerThreadsShPtr workers, const uint64_t chunkSize):
  workers_{ std::move(workers) },
  chunkSize_{chunkSize}
{ }


namespace
{
auto getChunk(Search::LogsPtr const& logs, Log::Key const& startPoint, const Search::Direction dir, const uint64_t chunkSize)
{
  switch(dir)
  {
    case Search::Direction::Forward:  return logs->withLock()->from(startPoint, chunkSize);
    case Search::Direction::Backward: return logs->withLock()->to  (startPoint, chunkSize);
  }
  BUT_ASSERT(!"invalid direction requested");
  throw std::logic_error{"invalid direction"};
}


But::Optional<Log::Key> lastKey(std::vector<Log> const& logs, const Search::Direction dir)
{
  if( logs.empty() )
    return {};
  switch(dir)
  {
    case Search::Direction::Forward:  return logs.back().key();
    case Search::Direction::Backward: return logs.front().key();
  }
  BUT_ASSERT(!"invalid direction requested");
  throw std::logic_error{"invalid direction"};
}


struct SharedState: But::Threading::BasicLockable<SharedState>,
                    But::Threading::LockProxyProvider<SharedState>
{
  explicit SharedState(But::NotNullShared<std::atomic<bool>> cancel,
                       But::NotNullShared<std::atomic<uint64_t>> requiredCompares,
                       But::NotNullShared<std::atomic<uint64_t>> comparesDone):
    cancel_{ std::move(cancel) },
    requiredCompares_{ std::move(requiredCompares) },
    comparesDone_{ std::move(comparesDone) }
  { }

  But::NotNullShared<std::atomic<bool>> cancel_;
  But::NotNullShared<std::atomic<uint64_t>> requiredCompares_;
  But::NotNullShared<std::atomic<uint64_t>> comparesDone_;
  But::Optional<Log::Key> keyFound_{};
  std::atomic<uint64_t> totalTasks_{0};
  std::atomic<uint64_t> doneTasks_{0};
};


auto canStopEarly(But::Optional<Log::Key> const& keyNow, const Search::Direction dir, SharedState const& state)
{
  if( state.cancel_->load() )
    return true;

  if(not keyNow)
    return false;

  const auto keyFound = state.withLock()->keyFound_;
  if(not keyFound)
    return false;

  switch(dir)
  {
    case Search::Direction::Forward:
         if( *keyFound <= *keyNow )
           return true;
         break;
    case Search::Direction::Backward:
         if( *keyNow <= *keyFound )
           return true;
         break;
  }
  return false;
}


struct ForwardSearchJob
{
  void operator()()
  {
    const auto taskGuard = But::makeGuard( [s=state_] { ++s->doneTasks_; } );
    BUT_ASSERT( not chunk_.empty() );
    if( canStopEarly(chunk_.front().key(), Search::Direction::Forward, *state_) )
      return;

    for(auto& e: chunk_)
    {
      try
      {
        if( state_->cancel_->load() )
          break;
        if( not query_(e) )
          continue;
        reportFinding( e.key() );
        break;
      }
      catch(...)
      { }
    }

    *state_->comparesDone_ += chunk_.size();
  }

  void reportFinding(Log::Key const& key)
  {
    auto ls = state_->withLock();
    if(not ls->keyFound_)
    {
      ls->keyFound_ = key;
      return;
    }
    if( key < *ls->keyFound_ )
    {
      ls->keyFound_ = key;
      return;
    }
  }

  Search::Query query_;
  But::NotNullShared<SharedState> state_;
  std::vector<Log> chunk_;
};


struct BackwardSearchJob
{
  void operator()()
  {
    const auto taskGuard = But::makeGuard( [s=state_] { ++s->doneTasks_; } );
    BUT_ASSERT( not chunk_.empty() );
    if( canStopEarly(chunk_.back().key(), Search::Direction::Backward, *state_) )
      return;

    for(auto it=chunk_.rbegin(); it!=chunk_.rend(); ++it)
    {
      try
      {
        if( state_->cancel_->load() )
          break;
        if( not query_(*it) )
          continue;
        reportFinding( it->key() );
        break;
      }
      catch(...)
      { }
    }

    *state_->comparesDone_ += chunk_.size();
  }

  void reportFinding(Log::Key const& key)
  {
    auto ls = state_->withLock();
    if(not ls->keyFound_)
    {
      ls->keyFound_ = key;
      return;
    }
    if( *ls->keyFound_ < key )
    {
      ls->keyFound_ = key;
      return;
    }
  }

  Search::Query query_;
  But::NotNullShared<SharedState> state_;
  std::vector<Log> chunk_;
};


struct SetSearchResult
{
  void operator()()
  {
    while( state_->totalTasks_ != state_->doneTasks_ && not state_->cancel_->load() )
      std::this_thread::yield();
    if( not state_->cancel_->load() )
      promise_->set_value( std::move(state_->keyFound_) );
    else
      promise_->set_value( But::Optional<Log::Key>{} );
  }

  But::NotNullShared<SharedState> state_;
  But::NotNullShared<std::promise<But::Optional<Log::Key>>> promise_;
};


size_t estimatedComparesCount(Search::LogsPtr const& logs, const Search::Direction dir, Log::Key const& startPoint)
{
  const auto ll = logs->withLock();
  const auto it = ll->lower_bound(startPoint);
  if( it == ll->end() )
    return 0;
  const auto index = ll->index( it->key() );
  const auto size = ll->size();
  BUT_ASSERT( size > index );

  switch(dir)
  {
    case Search::Direction::Forward: return size - index;
    case Search::Direction::Backward: return index;
  }
  BUT_ASSERT(!"unknown search direction");
  return 0;
}


void enqueueAllChunks(const std::weak_ptr<Utils::WorkerThreads> workersWeakPtr,
                      const Search::LogsPtr logs,
                      const Log::Key startPoint,
                      const Search::Direction dir,
                      const Search::Query q,
                      const But::NotNullShared<SharedState> state,
                      const uint64_t chunkSize,
                      const But::NotNullShared<std::promise<But::Optional<Log::Key>>> promise)
{
  auto workers = workersWeakPtr.lock();
  if(not workers)
    return;

  *state->requiredCompares_ = estimatedComparesCount(logs, dir, startPoint);
  uint64_t requiredCompares = 0;

  But::Optional<Log::Key> keyNow = startPoint;
  while(keyNow)
  {
    if( canStopEarly(keyNow, dir, *state) )
      break;
    auto chunk = getChunk(logs, *keyNow, dir, chunkSize);
    if( chunk.empty() )
      break;
    keyNow = lastKey(chunk, dir);
    const auto size = chunk.size();
    switch(dir)
    {
      case Search::Direction::Forward:  workers->enqueueUi( ForwardSearchJob{  q, state, std::move(chunk) } ); break;
      case Search::Direction::Backward: workers->enqueueUi( BackwardSearchJob{ q, state, std::move(chunk) } ); break;
    }
    ++state->totalTasks_;
    requiredCompares += size;
    if(size < chunkSize)
      break;
  }

  *state->requiredCompares_ = requiredCompares;

  workers->enqueueUi( SetSearchResult{state, promise} );
}
}


Search::Result Search::search(const LogsPtr logs, const Log::Key startPoint, const Direction dir, const Query q)
{
  auto cancel = But::makeSharedNN<std::atomic<bool>>(false);
  auto promise = But::makeSharedNN<std::promise<But::Optional<Log::Key>>>();
  auto requiredCompares = But::makeSharedNN<std::atomic<uint64_t>>(0);
  auto comparesDone = But::makeSharedNN<std::atomic<uint64_t>>(0);
  Search::Result result{ promise->get_future(), cancel, requiredCompares, comparesDone };
  auto state = But::makeSharedNN<SharedState>(cancel, requiredCompares, comparesDone);
  auto wwp = std::weak_ptr<Utils::WorkerThreads>{ workers_.underlyingPointer() };
  workers_->enqueueUi( [=, size=chunkSize_] { enqueueAllChunks(wwp, logs, startPoint, dir, q, state, size, promise); } );
  return result;
}

}
