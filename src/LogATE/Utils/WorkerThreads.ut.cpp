#include "LogATE/Utils/WorkerThreads.hpp"
#include <doctest/doctest.h>
#include <But/Threading/Event.hpp>

using LogATE::Utils::WorkerThreads;
//using LogATE::Tree::Node;

namespace
{
TEST_SUITE("Tree::Utils::WorkerThreads")
{

struct Fixture
{
  auto addBlocker(WorkerThreads& wt)
  {
    auto event = But::makeSharedNN<But::Threading::Event>();
    wt.enqueueBatch( [=] { event->wait( std::chrono::minutes{1} ); } );
    return event;
  }

  WorkerThreads wt_{1};
};


TEST_CASE_FIXTURE(Fixture, "0 threads is auto-mapped to 1 thread")
{
  const WorkerThreads wt{0};
  CHECK( wt.threads() == 1u );
}


TEST_CASE_FIXTURE(Fixture, "parallel processing does not break the queue")
{
  WorkerThreads wt{10};
  CHECK( wt.threads() == 10u );
  std::vector<But::NotNullShared<But::Threading::Event>> events;
  for(auto i=0u; i<wt.threads(); ++i)
    events.push_back( addBlocker(wt) );

  std::atomic<unsigned> counter{0};
  for(auto i=0; i<1000; ++i)
    wt.enqueueBatch( [&] { ++counter; } );

  CHECK( counter.load() == 0u );
  for(auto& e: events)
    e->set();
  wt.waitForAll();
  CHECK( counter.load() == 1000u );
}

}
}
