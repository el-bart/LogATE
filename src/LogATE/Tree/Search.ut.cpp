#include <doctest/doctest.h>
#include "LogATE/Tree/Search.hpp"
#include "LogATE/TestHelpers.ut.hpp"
#include "LogATE/TestPrints.ut.hpp"

using LogATE::Tree::Search;
using LogATE::Tree::Logs;
using LogATE::Log;

namespace
{
TEST_SUITE("Tree::Search")
{

struct Fixture
{
  auto populate(std::initializer_list<int> ns) const
  {
    std::vector<Log> out;
    out.reserve( ns.size() );
    for(auto e: ns)
    {
      auto json = nlohmann::json::object();
      json["answer"] = e;
      auto log = LogATE::makeLog( json.dump() );
      out.push_back(log);
      logs_->withLock()->insert( std::move(log) );
    }
    return out;
  }

  LogATE::Utils::WorkerThreadsShPtr workers_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>() };
  But::NotNullShared<Logs> logs_{ But::makeSharedNN<Logs>() };
  Search s{workers_, 5};
};


TEST_CASE_FIXTURE(Fixture, "searching within one block")
{
  SUBCASE("forward")
  {
  }
  SUBCASE("backward")
  {
  }
}


TEST_CASE_FIXTURE(Fixture, "searching within multiple blocks")
{
  SUBCASE("forward")
  {
  }
  SUBCASE("backward")
  {
  }
}


TEST_CASE_FIXTURE(Fixture, "canceling search")
{
  SUBCASE("forward")
  {
  }
  SUBCASE("backward")
  {
  }
}


TEST_CASE_FIXTURE(Fixture, "searching for a non-existing element")
{
  SUBCASE("forward")
  {
  }
  SUBCASE("backward")
  {
  }
}


TEST_CASE_FIXTURE(Fixture, "searcing with multiple matches returns first one")
{
  SUBCASE("forward")
  {
  }
  SUBCASE("backward")
  {
  }
}


TEST_CASE_FIXTURE(Fixture, "exception in query does not break the search")
{
  SUBCASE("forward")
  {
  }
  SUBCASE("backward")
  {
  }
}

}
}
