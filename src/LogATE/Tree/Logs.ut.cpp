#include <doctest/doctest.h>
#include "LogATE/Tree/Logs.hpp"

using LogATE::Tree::Logs;
using SN = LogATE::SequenceNumber;

namespace
{

TEST_SUITE("Tree::Logs")
{

struct Fixture
{
  auto makeLog(unsigned num) const
  {
    auto log = LogATE::json2log("{ 'foo': 'bar' }");
    log.sn_.value_ = num;
    return log;
  }

  Logs logs;
};

TEST_CASE_FIXTURE(Fixture, "empty set upon creation")
{
  CHECK( logs.withLock()->size() == 0u );
  CHECK( logs.withLock()->empty() );
}

TEST_CASE_FIXTURE(Fixture, "adding one element makes range usage")
{
  logs.insert( makeLog(42) );
  CHECK( not logs.withLock()->empty() );
  CHECK( logs.withLock()->size() == 1u );
}

}

}
