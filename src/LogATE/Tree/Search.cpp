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
  explicit SharedState(But::NotNullShared<std::atomic<bool>> cancel): cancel_{ std::move(cancel) } { }

  But::NotNullShared<std::atomic<bool>> cancel_;
  But::Optional<Log::Key> keyFound_{};
  std::atomic<uint64_t> totalTasks_{0};
  std::atomic<uint64_t> doneTasks_{0};
};


struct ForwardSearchJob
{
  void operator()()
  {
    const auto taskGuard = But::makeGuard( [s=state_] { ++s->doneTasks_; } );
    for(auto& e: chunk_)
    {
      try
      {
        if(*state_->cancel_)
          return;
        if( not query_(e) )
          continue;
        reportFinding( e.key() );
        return;
      }
      catch(...)
      { }
    }
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
    for(auto it=chunk_.rbegin(); it!=chunk_.rend(); ++it)
    {
      try
      {
        if(*state_->cancel_)
          return;
        if( not query_(*it) )
          continue;
        reportFinding( it->key() );
        return;
      }
      catch(...)
      { }
    }
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
}


Search::Result Search::search(const LogsPtr logs, const Log::Key startPoint, const Direction dir, const Query q)
{
  auto cancel = But::makeSharedNN<std::atomic<bool>>(false);
  auto promise = But::makeSharedNN<std::promise<But::Optional<Log::Key>>>();
  Search::Result result{ promise->get_future(), cancel };
  auto state = But::makeSharedNN<SharedState>(cancel);

  But::Optional<Log::Key> keyNow = startPoint;
  while(keyNow)
  {
    auto chunk = getChunk(logs, *keyNow, dir, chunkSize_);
    keyNow = lastKey(chunk, dir);
    const auto size = chunk.size();
    switch(dir)
    {
      case Search::Direction::Forward:  workers_->enqueueUi( ForwardSearchJob{  q, state, std::move(chunk) } ); break;
      case Search::Direction::Backward: workers_->enqueueUi( BackwardSearchJob{ q, state, std::move(chunk) } ); break;
    }
    ++state->totalTasks_;
    result.requiredCompares_ += size;
    if(size < chunkSize_)
    {
      keyNow.reset();
      break;
    }
  }

  workers_->enqueueUi( SetSearchResult{ state, std::move(promise) } );
  return result;
}

}
