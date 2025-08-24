#include "LogATE/Utils/WorkerThreads.hpp"
#include <But/Threading/Event.hpp>

namespace LogATE::Utils
{


WorkerThreads::~WorkerThreads()
{
  try
  {
    quit_ = true;
    // unblock all threads, if needed
    detail::Queue::lock_type lock{*q_};
    for(size_t i=0; i<threads(); ++i)
      q_->enqueueUi( TaskPtr{} );
  }
  catch(...)
  {
    BUT_ASSERT(!"failed to stop thread - givin up...");
    abort();
  }
}

void WorkerThreads::waitForAll()
{
  while(true)
  {
    auto waitAll = But::makeSharedNN<But::Threading::Event>();
    std::vector<But::NotNullShared<But::Threading::Event>> blocked;
    blocked.reserve( threads() );
    for(auto i=0u; i<threads(); ++i)
    {
      auto blocker = But::makeSharedNN<But::Threading::Event>();
      blocked.push_back(blocker);
      enqueueBatch( [blocker, waitAll] { blocker->set(); waitAll->wait(); } );
    }

    for(auto& b: blocked)
      b->wait();
    const auto hasMoreTasks = queued() > 0u;
    waitAll->set();
    if(not hasMoreTasks)
    {
      BUT_ASSERT( running() == threads() );
      return;
    }
  }
}


void WorkerThreads::enqueueTask(TaskPtr task)
{
  const detail::Queue::lock_type lock{*q_};
  q_->enqueueBatch( std::move(task) );
}


namespace
{
template<typename Q>
auto getTask(Q& q, std::atomic<unsigned>& running)
{
  typename Q::lock_type lock{q};
  q.waitForNonEmpty(lock);
  ++running;
  return std::make_pair( q.dequeue(), But::makeGuard( [&] { --running; } ) );
}
}

void WorkerThreads::processingLoop() noexcept
{
  while(not quit_)
  {
    try
    {
      auto [ cmd, incGuard ] = getTask(*q_, running_);
      if(cmd)
        cmd->run();
    }
    catch(...)
    {
      BUT_ASSERT(!"unexpected exception from processing thread - this should never happen");
    }
  }
}

}
