#include "LogATE/Utils/WorkerThreads.hpp"
#include <But/Threading/Event.hpp>

namespace LogATE::Utils
{

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
      enqueue( [&, i, waitAll] { done[i]->set(); waitAll->wait(); } );

    for(auto& e: done)
      e->wait();
    const auto hasMoreTasks = nonProcessed_ > threads();
    waitAll->set();
    if(not hasMoreTasks)
      return;
  }
}

}
