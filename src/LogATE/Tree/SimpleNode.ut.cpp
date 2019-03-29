#include <doctest/doctest.h>
#include "LogATE/Tree/SimpleNode.hpp"
#include "LogATE/Tree/TestHelpers.ut.hpp"
#include "LogATE/TestPrints.ut.hpp"

using LogATE::Tree::Node;
using LogATE::Tree::SimpleNode;
using LogATE::Tree::makeLog;
using LogATE::Tree::makeSns;
using LogATE::Tree::logs2sns;
using SN = LogATE::SequenceNumber;

namespace
{

TEST_SUITE("Tree::SimpleNode")
{

template<unsigned N>
struct ModFilter: SimpleNode
{
  ModFilter():
    SimpleNode(Node::Type{"test%2"}, Node::Name{"testing%2"}, {})
  { }

private:
  bool matches(LogATE::Log const& log) const override
  {
    return ( log.sn_.value_ % N ) == 0;
  }
};


struct Fixture
{
  auto allSns(Node const& n) const { return LogATE::Tree::allSns( n.logs() ); }
  auto allSns() const { return allSns(f2_); }

  ModFilter<2> f2_;
};


TEST_CASE_FIXTURE(Fixture, "filtering rule is applied")
{
  for(auto i: {1,2,3,4,5})
    f2_.insert( makeLog(i) );
  CHECK( allSns() == makeSns({2,4}) );
}

TEST_CASE_FIXTURE(Fixture, "cascading log through children works")
{
  REQUIRE( f2_.children().size() == 0 );
  f2_.add( But::makeUniqueNN<ModFilter<3>>() );
  REQUIRE( f2_.children().size() == 1 );
  f2_.add( But::makeUniqueNN<ModFilter<4>>() );
  REQUIRE( f2_.children().size() == 2 );

  for(auto i=0; i<15; ++i)
    f2_.insert( makeLog(i) );

  CHECK( allSns() == makeSns({0,2,4,6,8,10,12,14}) );
  CHECK( allSns(*f2_.children()[0]) == makeSns({0,6,12}) );
  CHECK( allSns(*f2_.children()[1]) == makeSns({0,4,8,12}) );
}

TEST_CASE_FIXTURE(Fixture, "adding child in a middle of a run adds all the logs to its filter base")
{
  for(auto i=0; i<15; ++i)
    f2_.insert( makeLog(i) );

  REQUIRE( f2_.children().size() == 0 );
  f2_.add( But::makeUniqueNN<ModFilter<4>>() );
  REQUIRE( f2_.children().size() == 1 );
  CHECK( allSns(*f2_.children()[0]) == makeSns({0,4,8,12}) );
}


struct AlwaysThrow: SimpleNode
{
  AlwaysThrow():
    SimpleNode(Node::Type{"thrower"}, Node::Name{"mr."}, {})
  { }

private:
  bool matches(LogATE::Log const&) const override
  {
    throw std::runtime_error{"expected"};
  }
};

TEST_CASE_FIXTURE(Fixture, "exception in passing through to one child does not affect others")
{
  REQUIRE( f2_.children().size() == 0 );
  f2_.add( But::makeUniqueNN<AlwaysThrow>() );
  REQUIRE( f2_.children().size() == 1 );
  f2_.add( But::makeUniqueNN<ModFilter<4>>() );
  REQUIRE( f2_.children().size() == 2 );

  for(auto i=0; i<15; ++i)
    f2_.insert( makeLog(i) );

  CHECK( allSns() == makeSns({0,2,4,6,8,10,12,14}) );
  CHECK( allSns(*f2_.children()[0]) == makeSns({}) );
  CHECK( allSns(*f2_.children()[1]) == makeSns({0,4,8,12}) );
}

}

}
