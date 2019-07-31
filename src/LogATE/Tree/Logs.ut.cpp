#include <doctest/doctest.h>
#include "LogATE/Tree/Logs.hpp"
#include "LogATE/Tree/TestHelpers.ut.hpp"
#include "LogATE/TestPrints.ut.hpp"

using LogATE::Log;
using LogATE::Tree::Logs;
using LogATE::Tree::makeLog;
using LogATE::Tree::makeSns;
using LogATE::Tree::logs2sns;

namespace
{
TEST_SUITE("Tree::Logs")
{

struct Fixture
{
  auto allSns() const { return LogATE::Tree::allSns(logs_); }

  Logs logs_;
};


TEST_CASE_FIXTURE(Fixture, "empty set upon creation")
{
  CHECK( logs_.withLock()->size() == 0u );
  CHECK( logs_.withLock()->empty() );
}


TEST_CASE_FIXTURE(Fixture, "adding one element makes range usage")
{
  const auto tmp = makeLog(42);
  logs_.withLock()->insert(tmp);
  CHECK( not logs_.withLock()->empty() );
  CHECK( logs_.withLock()->size() == 1u );
  CHECK( logs_.withLock()->last().sequenceNumber() == tmp.sequenceNumber() );
  CHECK( logs_.withLock()->first().sequenceNumber() == tmp.sequenceNumber() );
}


TEST_CASE_FIXTURE(Fixture, "elements added in order are ordered correctly")
{
  for(auto i=0; i<10; ++i)
  {
    const auto tmp = makeLog(i);
    logs_.withLock()->insert(tmp);
    CHECK( not logs_.withLock()->empty() );
    CHECK( logs_.withLock()->size() == i+1 );
    CHECK( logs_.withLock()->last().sequenceNumber() == tmp.sequenceNumber() );
  }
}


TEST_CASE_FIXTURE(Fixture, "elements added in different order are auto-ordered correctly")
{
  const auto tmp1 = makeLog(41);
  const auto tmp2 = makeLog(43);
  logs_.withLock()->insert(tmp2);
  logs_.withLock()->insert(tmp1);
  CHECK( not logs_.withLock()->empty() );
  CHECK( logs_.withLock()->size() == 2u );
  CHECK( logs_.withLock()->last().sequenceNumber() == tmp2.sequenceNumber() );
  CHECK( logs_.withLock()->first().sequenceNumber() == tmp1.sequenceNumber() );
}


TEST_CASE_FIXTURE(Fixture, "prunning elements until given value")
{
  for(auto sn: {2,3,5,7,8,9})
    logs_.withLock()->insert( makeLog(sn) );
  CHECK( logs_.withLock()->pruneUpTo(Log::Key{"8"}) == 4 );
  CHECK( allSns() == makeSns({8,9}) );
}


TEST_CASE_FIXTURE(Fixture, "prunning elements until non existing low value will not touch collection")
{
  for(auto sn: {2,3,5,7,8,9})
    logs_.withLock()->insert( makeLog(sn) );
  CHECK( logs_.withLock()->pruneUpTo(Log::Key{"1"}) == 0 );
  CHECK( allSns() == makeSns({2,3,5,7,8,9}) );
}


TEST_CASE_FIXTURE(Fixture, "prunning elements until non existing high value will clean collection")
{
  for(auto sn: {2,3,5,7,8,9})
    logs_.withLock()->insert( makeLog(sn) );
  CHECK( logs_.withLock()->pruneUpTo(Log::Key{"42"}) == 6 );
  CHECK( logs_.withLock()->empty() );
}


TEST_CASE_FIXTURE(Fixture, "getting given range")
{
  for(auto sn: {2,3,5,7,8,9})
    logs_.withLock()->insert( makeLog(sn) );
  CHECK( logs2sns( logs_.withLock()->range(Log::Key{"3"}, Log::Key{"8"})  ) == makeSns({3,5,7}) );
  CHECK( logs2sns( logs_.withLock()->range(Log::Key{"1"}, Log::Key{"8"})  ) == makeSns({2,3,5,7}) );
  CHECK( logs2sns( logs_.withLock()->range(Log::Key{"3"}, Log::Key{"42"}) ) == makeSns({3,5,7,8,9}) );
  CHECK( logs2sns( logs_.withLock()->range(Log::Key{"3"}, Log::Key{"6"})  ) == makeSns({3,5}) );
}


TEST_CASE_FIXTURE(Fixture, "getting count starting with a given position")
{
  for(auto sn: {2,3,5,7,8,9})
    logs_.withLock()->insert( makeLog(sn) );
  CHECK( logs2sns( logs_.withLock()->from(Log::Key{"3"}, 0)  ) == makeSns({}) );
  CHECK( logs2sns( logs_.withLock()->from(Log::Key{"2"}, 1)  ) == makeSns({2}) );
  CHECK( logs2sns( logs_.withLock()->from(Log::Key{"9"}, 1)  ) == makeSns({9}) );
  CHECK( logs2sns( logs_.withLock()->from(Log::Key{"1"}, 1)  ) == makeSns({2}) );
  CHECK( logs2sns( logs_.withLock()->from(Log::Key{"6"}, 1)  ) == makeSns({7}) );
  CHECK( logs2sns( logs_.withLock()->from(Log::Key{"4"}, 2)  ) == makeSns({5,7}) );
  CHECK( logs2sns( logs_.withLock()->from(Log::Key{"9"}, 1)  ) == makeSns({9}) );
  CHECK( logs2sns( logs_.withLock()->from(Log::Key{"3"}, 99)  ) == makeSns({3,5,7,8,9}) );
  CHECK( logs2sns( logs_.withLock()->from(Log::Key{"13"}, 5)  ) == makeSns({}) );
  CHECK( logs2sns( logs_.withLock()->from(Log::Key{"0"}, 99)  ) == makeSns({2,3,5,7,8,9}) );
}


TEST_CASE_FIXTURE(Fixture, "getting count ending with a given position")
{
  for(auto sn: {2,3,5,7,8,9})
    logs_.withLock()->insert( makeLog(sn) );
  CHECK( logs2sns( logs_.withLock()->to(Log::Key{"3"}, 0)  ) == makeSns({}) );
  CHECK( logs2sns( logs_.withLock()->to(Log::Key{"99"}, 0)  ) == makeSns({}) );
  CHECK( logs2sns( logs_.withLock()->to(Log::Key{"2"}, 1)  ) == makeSns({2}) );
  CHECK( logs2sns( logs_.withLock()->to(Log::Key{"9"}, 1)  ) == makeSns({9}) );
  CHECK( logs2sns( logs_.withLock()->to(Log::Key{"0"}, 1)  ) == makeSns({}) );
  CHECK( logs2sns( logs_.withLock()->to(Log::Key{"13"}, 1)  ) == makeSns({9}) );
  CHECK( logs2sns( logs_.withLock()->to(Log::Key{"9"}, 2)  ) == makeSns({8,9}) );
  CHECK( logs2sns( logs_.withLock()->to(Log::Key{"9"}, 6)  ) == makeSns({2,3,5,7,8,9}) );
  CHECK( logs2sns( logs_.withLock()->to(Log::Key{"9"}, 666)  ) == makeSns({2,3,5,7,8,9}) );
  CHECK( logs2sns( logs_.withLock()->to(Log::Key{"5"}, 666)  ) == makeSns({2,3,5}) );
  CHECK( logs2sns( logs_.withLock()->to(Log::Key{"5"}, 2)  ) == makeSns({3,5}) );
}


TEST_CASE_FIXTURE(Fixture, "finding element in set")
{
  for(auto sn: {2,3,5,9})
    logs_.withLock()->insert( makeLog(sn) );
  auto ll = logs_.withLock();
  SUBCASE("first")
  {
    const auto it = ll->find( Log::Key{"2"} );
    REQUIRE( it != ll->end() );
    CHECK( it->key() == Log::Key{"2"} );
  }
  SUBCASE("middle")
  {
    const auto it = ll->find( Log::Key{"5"} );
    REQUIRE( it != ll->end() );
    CHECK( it->key() == Log::Key{"5"} );
  }
  SUBCASE("last")
  {
    const auto it = ll->find( Log::Key{"9"} );
    REQUIRE( it != ll->end() );
    CHECK( it->key() == Log::Key{"9"} );
  }
  SUBCASE("non-existing - middle")
  {
    const auto it = ll->find( Log::Key{"4"} );
    REQUIRE( it == ll->end() );
  }
  SUBCASE("non-existing - before first")
  {
    const auto it = ll->find( Log::Key{"1"} );
    REQUIRE( it == ll->end() );
  }
  SUBCASE("non-existing - after last")
  {
    const auto it = ll->find( Log::Key{"10"} );
    REQUIRE( it == ll->end() );
  }
  SUBCASE("const")
  {
    const auto llc = std::move(ll);
    const auto it = llc->find( Log::Key{"2"} );
    REQUIRE( it != llc->end() );
    CHECK( it->key() == Log::Key{"2"} );
  }
}

}
}
