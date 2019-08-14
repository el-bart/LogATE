#include <doctest/doctest.h>
#include "LogATE/Tree/SimpleNode.hpp"
#include "LogATE/TestHelpers.ut.hpp"
#include "LogATE/TestPrints.ut.hpp"

using LogATE::Tree::Node;
using LogATE::Tree::SimpleNode;
using LogATE::makeAnnotatedLog;
using LogATE::makeSns;
using LogATE::logs2sns;
using SN = LogATE::SequenceNumber;

namespace
{
TEST_SUITE("Tree::SimpleNode")
{

template<unsigned N>
struct ModFilter: SimpleNode
{
  explicit ModFilter(LogATE::Utils::WorkerThreadsShPtr workers):
    SimpleNode{ std::move(workers), Node::Type{"test%2"}, Node::Name{"testing%2"}, {} }
  { }

private:
  bool matches(LogATE::AnnotatedLog const& log) const override
  {
    return ( log.log().sequenceNumber().value_ % N ) == 0;
  }
};


struct Fixture
{
  auto allSns(Node const& n) const { return LogATE::allSns( n.logs() ); }
  auto allSns() const { return allSns(f2_); }

  LogATE::Utils::WorkerThreadsShPtr workers_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>() };
  ModFilter<2> f2_{workers_};
};


TEST_CASE_FIXTURE(Fixture, "filtering rule is applied")
{
  CHECK( f2_.insert( makeAnnotatedLog(1) ) == false );
  CHECK( f2_.insert( makeAnnotatedLog(2) ) == true );
  CHECK( f2_.insert( makeAnnotatedLog(3) ) == false );
  CHECK( f2_.insert( makeAnnotatedLog(4) ) == true );
  CHECK( f2_.insert( makeAnnotatedLog(5) ) == false );
  CHECK( allSns() == makeSns({2,4}) );
}

TEST_CASE_FIXTURE(Fixture, "cascading log through children works")
{
  REQUIRE( f2_.children().size() == 0 );
  f2_.add( But::makeUniqueNN<ModFilter<3>>(workers_) );
  workers_->waitForAll();
  REQUIRE( f2_.children().size() == 1 );
  f2_.add( But::makeUniqueNN<ModFilter<4>>(workers_) );
  workers_->waitForAll();
  REQUIRE( f2_.children().size() == 2 );

  for(auto i=0; i<15; ++i)
    f2_.insert( makeAnnotatedLog(i) );

  CHECK( allSns() == makeSns({0,2,4,6,8,10,12,14}) );
  CHECK( allSns(*f2_.children()[0]) == makeSns({0,6,12}) );
  CHECK( allSns(*f2_.children()[1]) == makeSns({0,4,8,12}) );
}

TEST_CASE_FIXTURE(Fixture, "adding child in a middle of a run adds all the logs to its filter base")
{
  for(auto i=0; i<15; ++i)
    f2_.insert( makeAnnotatedLog(i) );

  REQUIRE( f2_.children().size() == 0 );
  f2_.add( But::makeUniqueNN<ModFilter<4>>(workers_) );
  workers_->waitForAll();
  REQUIRE( f2_.children().size() == 1 );
  CHECK( allSns(*f2_.children()[0]) == makeSns({0,4,8,12}) );
}


struct AlwaysThrow: SimpleNode
{
  explicit AlwaysThrow(LogATE::Utils::WorkerThreadsShPtr workers):
    SimpleNode{ std::move(workers), Node::Type{"thrower"}, Node::Name{"mr."}, {} }
  { }

private:
  bool matches(LogATE::AnnotatedLog const&) const override
  {
    throw std::runtime_error{"expected"};
  }
};

TEST_CASE_FIXTURE(Fixture, "exception in passing through to one child does not affect others")
{
  REQUIRE( f2_.children().size() == 0 );
  f2_.add( But::makeUniqueNN<AlwaysThrow>(workers_) );
  workers_->waitForAll();
  REQUIRE( f2_.children().size() == 1 );
  f2_.add( But::makeUniqueNN<ModFilter<4>>(workers_) );
  workers_->waitForAll();
  REQUIRE( f2_.children().size() == 2 );

  for(auto i=0; i<15; ++i)
    f2_.insert( makeAnnotatedLog(i) );

  CHECK( allSns() == makeSns({0,2,4,6,8,10,12,14}) );
  CHECK( allSns(*f2_.children()[0]) == makeSns({}) );
  CHECK( allSns(*f2_.children()[1]) == makeSns({0,4,8,12}) );
}

TEST_CASE_FIXTURE(Fixture, "cascading log through children works")
{
  f2_.add( But::makeUniqueNN<ModFilter<3>>(workers_) );
  f2_.add( But::makeUniqueNN<ModFilter<4>>(workers_) );
  workers_->waitForAll();
  REQUIRE( f2_.children().size() == 2 );

  SUBCASE("removal of existing child works")
  {
    const auto node = f2_.children()[0];
    CHECK( f2_.remove(node).get() == node.get() );
    CHECK( f2_.children().size() == 1 );
  }

  SUBCASE("double-removal of a child does not do anything")
  {
    const auto node = f2_.children()[0];
    CHECK( f2_.remove(node).get() == node.get() );
    CHECK( f2_.remove(node).get() == nullptr );
    CHECK( f2_.children().size() == 1 );
  }
}

}
}
