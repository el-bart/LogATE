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
    std::vector<But::NotNullShared<But::Threading::Event>> done;
    done.reserve( threads() );
    for(auto i=0u; i<threads(); ++i)
      done.push_back( But::makeSharedNN<But::Threading::Event>() );
    auto waitAll = But::makeSharedNN<But::Threading::Event>();

    BUT_ASSERT( done.size() == threads() );
    for(auto i=0u; i<threads(); ++i)
      enqueueBatch( [doneMark=done[i], waitAll] { doneMark->set(); waitAll->wait(); } );

    for(auto& e: done)
      e->wait();
    const auto hasMoreTasks = running() > threads();
    waitAll->set();
    if(not hasMoreTasks)
      return;
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
auto getTask(Q& q)
{
  typename Q::lock_type lock{q};
  q.waitForNonEmpty(lock);
  return q.dequeue();
}
}

void WorkerThreads::processingLoop() noexcept
{
  while(not quit_)
  {
    try
    {
      auto cmd = getTask(*q_);
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
