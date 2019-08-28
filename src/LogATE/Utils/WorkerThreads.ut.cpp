#include "LogATE/Utils/WorkerThreads.hpp"
#include <doctest/doctest.h>
#include <But/Threading/Event.hpp>
#include <set>
#include <mutex>
#include <thread>

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


TEST_CASE_FIXTURE(Fixture, "return value is respected")
{
  SUBCASE("return value")
  {
    auto ret = wt_.enqueueBatch( [] { return 42; } );
    CHECK( ret.get() == 42 );
  }
  SUBCASE("return void value")
  {
    auto ret = wt_.enqueueBatch( []{} );
    REQUIRE( ( std::is_same<void, decltype(ret.get())>::value ) );
    ret.get();
  }
  SUBCASE("return exception")
  {
    auto ret = wt_.enqueueBatch( [] { throw std::string{"foo-bar"}; } );
    try
    {
      ret.get();
      FAIL("exception has not been propagated");
    }
    catch(std::string const& str)
    {
      CHECK( str == "foo-bar" );
    }
  }
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


TEST_CASE_FIXTURE(Fixture, "all threads are used, if enough tasks are provided")
{
  WorkerThreads wt{3};
  But::Threading::Event event;
  std::atomic<unsigned> waiting{0};
  std::atomic<unsigned> ready{0};
  for(auto i=0u; i<wt.threads(); ++i)
    wt.enqueueBatch( [&] { ++waiting; event.wait(); ++ready; } );

  while( waiting.load() != wt.threads() )
    std::this_thread::yield();

  CHECK( ready.load() == 0u );
  event.set();
  wt.waitForAll();
  CHECK( ready.load() == wt.threads() );
}


TEST_CASE_FIXTURE(Fixture, "queues have proper priorities")
{
  auto event = addBlocker(wt_);
  auto guard = [=] { event->set(); };

  const auto type = LogATE::Tree::NodeType{"some type"};
  const auto name1 = LogATE::Tree::NodeName{"some name #1"};
  const auto name2 = LogATE::Tree::NodeName{"some name #2"};

  REQUIRE( wt_.threads() == 1u );
  std::vector<std::string> execOrder;
  for(auto i=0u; i<3u; ++i)
  {
    wt_.enqueueBatch(               [i,&execOrder] { execOrder.push_back( std::to_string(i) + "/batch" ); } );
    wt_.enqueueUi(                  [i,&execOrder] { execOrder.push_back( std::to_string(i) + "/ui" ); } );
    wt_.enqueueFilter( type, name1, [i,&execOrder] { execOrder.push_back( std::to_string(i) + "/filter1" ); } );
    wt_.enqueueFilter( type, name2, [i,&execOrder] { execOrder.push_back( std::to_string(i) + "/filter2" ); } );
  }

  event->set();
  wt_.waitForAll();
  const auto expectedOrder = std::vector<std::string>{
    "0/ui",
    "1/ui",
    "2/ui",
    "0/filter1",
    "0/filter2",
    "1/filter1",
    "1/filter2",
    "2/filter1",
    "2/filter2",
    "0/batch",
    "1/batch",
    "2/batch"
  };
  REQUIRE( execOrder.size() == expectedOrder.size() );
  for(auto i=0u; i<execOrder.size(); ++i)
    CHECK( execOrder[i] == expectedOrder[i] );
}


TEST_CASE_FIXTURE(Fixture, "rescheduling is done when new tasks arrive with higher priority")
{
  auto event = addBlocker(wt_);
  auto guard = [=] { event->set(); };

  const auto type = LogATE::Tree::NodeType{"some type"};
  const auto name = LogATE::Tree::NodeName{"some name"};

  REQUIRE( wt_.threads() == 1u );
  std::vector<std::string> execOrder;
  wt_.enqueueBatch(              [&execOrder,event] { execOrder.push_back( std::to_string(0) + "/batch" ); event->wait(); } );
  wt_.enqueueBatch(              [&execOrder] { execOrder.push_back( std::to_string(1) + "/batch" ); } );
  wt_.enqueueFilter( type, name, [&execOrder] { execOrder.push_back( std::to_string(2) + "/filter" ); } );
  wt_.enqueueUi(                 [&execOrder] { execOrder.push_back( std::to_string(3) + "/ui" ); } );
  wt_.enqueueFilter( type, name, [&execOrder] { execOrder.push_back( std::to_string(4) + "/filter" ); } );
  wt_.enqueueUi(                 [&execOrder] { execOrder.push_back( std::to_string(5) + "/ui" ); } );

  event->set();
  wt_.waitForAll();
  const auto expectedOrder = std::vector<std::string>{
    "0/batch",
    "3/ui",
    "5/ui",
    "2/filter",
    "4/filter",
    "1/batch"
  };
  REQUIRE( execOrder.size() == expectedOrder.size() );
  for(auto i=0u; i<execOrder.size(); ++i)
    CHECK( execOrder[i] == expectedOrder[i] );
}

}
}
