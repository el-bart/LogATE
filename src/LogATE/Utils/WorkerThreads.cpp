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
    Queue::lock_type lock{q_};
    for(size_t i=0; i<threads(); ++i)
      q_.push( TaskPtr{} );
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
    std::vector<But::NotNullUnique<But::Threading::Event>> done;
    done.reserve( threads() );
    for(auto i=0u; i<threads(); ++i)
      done.push_back( But::makeUniqueNN<But::Threading::Event>() );
    auto waitAll = But::makeSharedNN<But::Threading::Event>();

    BUT_ASSERT( done.size() == threads() );
    for(auto i=0u; i<threads(); ++i)
      enqueueBatch( [&, i, waitAll] { done[i]->set(); waitAll->wait(); } );

    for(auto& e: done)
      e->wait();
    const auto hasMoreTasks = nonProcessed_ > threads();
    waitAll->set();
    if(not hasMoreTasks)
      return;
  }
}


void WorkerThreads::enqueueTask(TaskPtr task)
{
  const Queue::lock_type lock{q_};
  q_.push( std::move(task) );
}


namespace
{
template<typename Q>
auto getTask(Q& q)
{
  typename Q::lock_type lock{q};
  q.waitForNonEmpty(lock);
  auto cmd = std::move( q.top() );
  q.pop();
  return cmd;
}
}

void WorkerThreads::processingLoop() noexcept
{
  while(not quit_)
  {
    try
    {
      auto cmd = getTask(q_);
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
