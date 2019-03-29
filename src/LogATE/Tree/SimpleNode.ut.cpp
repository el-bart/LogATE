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
  auto allSns() const { return LogATE::Tree::allSns( f2_.logs() ); }

  ModFilter<2> f2_;
};


TEST_CASE_FIXTURE(Fixture, "filtering rule is applied")
{
  for(auto i: {1,2,3,4,5})
    f2_.insert( makeLog(i) );
  CHECK( allSns() == makeSns({2,4}) );
}

TEST_CASE_FIXTURE(Fixture, "adding children works")
{
  CHECK( f2_.children().size() == 0 );
  f2_.add( But::makeUniqueNN<ModFilter<4>>() );
  CHECK( f2_.children().size() == 1 );
  f2_.add( But::makeUniqueNN<ModFilter<3>>() );
  CHECK( f2_.children().size() == 2 );
}

TEST_CASE_FIXTURE(Fixture, "filtering is cascading to children")
{
  // TODO
}

}

}
