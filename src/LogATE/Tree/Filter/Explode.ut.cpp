#include <doctest/doctest.h>
#include "LogATE/Tree/Filter/Explode.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/Tree/TestHelpers.ut.hpp"
#include "LogATE/TestPrints.ut.hpp"
#include <vector>
#include <map>

using LogATE::Log;
using LogATE::SequenceNumber;
using LogATE::json2log;
using LogATE::Tree::allSns;
using LogATE::Tree::Node;
using LogATE::Tree::Path;
using LogATE::Tree::Filter::Explode;

namespace
{
TEST_SUITE("Tree::Filter::Explode")
{

struct Fixture
{
  auto extractLogs(Explode const& e) const
  {
    std::map<Node::Name, std::vector<SequenceNumber>> out;
    for(auto node: e.children())
      out[ node->name() ] = allSns( node->logs() );
    return out;
  }

  auto extractLogs() const { return extractLogs(explode_); }

  Log makeLog(const unsigned sn, std::string const& json) const
  {
    auto log = json2log(json);
    log.sn_ = SequenceNumber{sn};
    return log;
  }

  auto name(std::string const& name) const { return Explode::Name{name}; }
  template<typename ...Args>
  auto sns(Args... args) { return std::vector<SequenceNumber>{args.sn_...}; }

  LogATE::Utils::WorkerThreadsShPtr workers_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>() };
  Explode explode_{ workers_, Explode::Name{"foo"}, Path{{".", "foo"}} };
  const Log log1_{ makeLog(1, R"( { "foo": "xxx", "bar": 42 } )") };
  const Log log2_{ makeLog(2, R"( { "foo": "yyy", "bar": 44 } )") };
  const Log log3_{ makeLog(3, R"( { "foo": "xxx", "bar": 46 } )") };
  const Log log4_{ makeLog(4, R"( { "xxx": "xxx", "bar": 48 } )") };
};


TEST_CASE_FIXTURE(Fixture, "there is always a default child for getting all unmatched logs")
{
  const auto out = extractLogs();
  CHECK( out.count( Explode::nonMatchingChildName() ) == 1 );
}

TEST_CASE_FIXTURE(Fixture, "trimming search field")
{
  const auto tf = explode_.trimFields();
  REQUIRE( tf.size() == 1 );
  CHECK( tf[0] == Path{{".", "foo"}} );
}

TEST_CASE_FIXTURE(Fixture, "hitting one destination")
{
  explode_.insert(log1_);
  explode_.insert(log3_);
  const auto out = extractLogs();
  REQUIRE( out.size() == 2 );
  CHECK( out.at( Explode::nonMatchingChildName() ) == sns() );
  CHECK( out.at( name("xxx") ) == sns(log1_, log3_) );
}

TEST_CASE_FIXTURE(Fixture, "new value creates new destination")
{
  explode_.insert(log1_);
  explode_.insert(log2_);
  explode_.insert(log3_);
  const auto out = extractLogs();
  REQUIRE( out.size() == 3 );
  CHECK( out.at( Explode::nonMatchingChildName() ) == sns() );
  CHECK( out.at( name("xxx") ) == sns(log1_, log3_) );
  CHECK( out.at( name("yyy") ) == sns(log2_) );
}

TEST_CASE_FIXTURE(Fixture, "logs w/o required fieled go to default location")
{
  explode_.insert(log4_);
  const auto out = extractLogs();
  REQUIRE( out.size() == 1 );
  CHECK( out.at( Explode::nonMatchingChildName() ) == sns(log4_) );
}

TEST_CASE_FIXTURE(Fixture, "field can be specified with a relative path")
{
  Explode explode{ workers_, Explode::Name{"foo"}, Path{{"foo"}} };
  explode.insert(log1_);
  explode.insert(log2_);
  explode.insert(log3_);
  const auto out = extractLogs(explode);
  REQUIRE( out.size() == 3 );
  CHECK( out.at( Explode::nonMatchingChildName() ) == sns() );
  CHECK( out.at( name("xxx") ) == sns(log1_, log3_) );
  CHECK( out.at( name("yyy") ) == sns(log2_) );
}

TEST_CASE_FIXTURE(Fixture, "ambigous relative path adds logs to multiple cathegories")
{
  const auto log = makeLog(1, R"( { "foo": "xxx", "bar": { "foo": "yyy" } } )");
  Explode explode{ workers_, Explode::Name{"foo"}, Path{{"foo"}} };
  explode.insert(log);
  const auto out = extractLogs(explode);
  REQUIRE( out.size() == 3 );
  CHECK( out.at( Explode::nonMatchingChildName() ) == sns() );
  CHECK( out.at( name("xxx") ) == sns(log) );
  CHECK( out.at( name("yyy") ) == sns(log) );
}

TEST_CASE_FIXTURE(Fixture, "explicit addition/removal of a child fails")
{
  using LogATE::Tree::Filter::AcceptAll;
  CHECK_THROWS_AS( explode_.add( But::makeUniqueNN<AcceptAll>(workers_, name("xxx")) ), Explode::ExplicitNodeAddNotSupported );
  CHECK_THROWS_AS( explode_.remove( But::makeUniqueNN<AcceptAll>(workers_, name("xxx")) ), Explode::ExplicitNodeRemoveNotSupported );
}

}
}
