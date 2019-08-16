#include <doctest/doctest.h>
#include "LogATE/Tree/Filter/BinarySplit.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/Tree/SimpleNode.hpp"
#include "LogATE/TestHelpers.ut.hpp"
#include "LogATE/TestPrints.ut.hpp"

using LogATE::Log;
using LogATE::AnnotatedLog;
using LogATE::SequenceNumber;
using LogATE::allSns;
using LogATE::makeLog;
using LogATE::Tree::Node;
using LogATE::Tree::Path;
using LogATE::Tree::SimpleNode;
using LogATE::Tree::Filter::BinarySplit;
using LogATE::Tree::Filter::AcceptAll;

namespace
{
TEST_SUITE("Tree::Filter::BinarySplit")
{

struct OddFilter: SimpleNode
{
  explicit OddFilter(LogATE::Utils::WorkerThreadsShPtr workers):
    SimpleNode{ std::move(workers), Node::Type{"OddFilter"}, Node::Name{"testing%2"}, {} }
  { }

private:
  bool matches(LogATE::AnnotatedLog const& log) const override
  {
    return ( log.log().sequenceNumber().value_ % 2 ) != 0;
  }
};


struct Fixture
{
  auto extractLogs(BinarySplit const& bs) const
  {
    std::vector<std::vector<SequenceNumber>> out(2);
    const auto children = bs.children();
    REQUIRE( children.size() == 2 );
    for(auto i=0; i<2; ++i)
      out[i] = allSns( *children[i]->clogs() );
    return out;
  }
  auto extractLogs() const { return extractLogs(bs_); }

  template<typename ...Args>
  auto sns(Args... args) { return std::vector<SequenceNumber>{args.log().sequenceNumber()...}; }

  LogATE::Utils::WorkerThreadsShPtr workers_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>() };
  BinarySplit bs_{ workers_, Node::Name{"foo"}, But::makeUniqueNN<OddFilter>(workers_) };

  const AnnotatedLog log1_{ makeLog(1, R"( { "foo": "xxx", "bar": 42 } )") };
  const AnnotatedLog log2_{ makeLog(2, R"( { "foo": "yyy", "bar": 44 } )") };
  const AnnotatedLog log3_{ makeLog(3, R"( { "foo": "xxx", "bar": 46 } )") };
  const AnnotatedLog log4_{ makeLog(4, R"( { "xxx": "xxx", "bar": 48 } )") };
};


TEST_CASE_FIXTURE(Fixture, "set of children is constant")
{
  SUBCASE("empty filter")
  {
    const auto c = bs_.children();
    REQUIRE( c.size() == 2 );
    CHECK( c[0]->name() == Node::Name{"testing%2"} );
    CHECK( c[1]->name() == BinarySplit::nonMatchingChildName() );
  }

  SUBCASE("filter with some data")
  {
    for(auto& log: {log1_, log2_, log3_, log4_})
      bs_.insert(log);

    const auto c = bs_.children();
    REQUIRE( c.size() == 2 );
    CHECK( c[0]->name() == Node::Name{"testing%2"} );
    CHECK( c[1]->name() == BinarySplit::nonMatchingChildName() );
  }
}


TEST_CASE_FIXTURE(Fixture, "no fields are trimmed")
{
  const auto tf = bs_.trimFields();
  REQUIRE( tf.empty() );
}


TEST_CASE_FIXTURE(Fixture, "all logs are kept in master node")
{
  bs_.insert(log1_);
  bs_.insert(log2_);
  bs_.insert(log3_);
  bs_.insert(log4_);
  CHECK( bs_.clogs()->withLock()->size() == 4 );
}


TEST_CASE_FIXTURE(Fixture, "logs are split into matched and unmatched categories")
{
  bs_.insert(log1_);
  bs_.insert(log2_);
  bs_.insert(log3_);
  bs_.insert(log4_);
  const auto out = extractLogs();
  REQUIRE( out.size() == 2 );
  CHECK( out[0] == sns(log1_, log3_) );
  CHECK( out[1] == sns(log2_, log4_) );
}


TEST_CASE_FIXTURE(Fixture, "explicit addition/removal of a child fails")
{
  CHECK_THROWS_AS( bs_.add( But::makeUniqueNN<AcceptAll>(workers_, Node::Name{"xxx"}) ), BinarySplit::ExplicitNodeAddNotSupported );
  CHECK_THROWS_AS( bs_.remove( But::makeUniqueNN<AcceptAll>(workers_, Node::Name{"xxx"}) ), BinarySplit::ExplicitNodeRemoveNotSupported );
}


TEST_CASE_FIXTURE(Fixture, "insert() always adds a node")
{
  for(auto& log: {log1_, log2_, log3_, log4_})
    CHECK( bs_.insert(log) == true );
  auto sum = 0u;
  for(auto& n: extractLogs())
    sum += n.size();
  CHECK( sum == 4 );
}

}
}
