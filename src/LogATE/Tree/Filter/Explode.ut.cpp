#include <doctest/doctest.h>
#include "LogATE/Tree/Filter/Explode.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/Tree/TestHelpers.ut.hpp"
#include "LogATE/TestPrints.ut.hpp"
#include <vector>
#include <map>

using LogATE::Log;
using LogATE::AnnotatedLog;
using LogATE::SequenceNumber;
using LogATE::Tree::allSns;
using LogATE::Tree::Node;
using LogATE::Tree::Path;
using LogATE::Tree::Filter::Explode;
using LogATE::Tree::Filter::AcceptAll;

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
    return LogATE::Log{ SequenceNumber{sn}, json };
  }

  auto name(std::string const& name) const { return Explode::Name{name}; }
  template<typename ...Args>
  auto sns(Args... args) { return std::vector<SequenceNumber>{args.log_.sequenceNumber()...}; }

  LogATE::Utils::WorkerThreadsShPtr workers_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>() };
  const Path defaultPath_{ Path::parse(".foo") };
  Explode explode_{ workers_, Explode::Name{"foo"}, defaultPath_ };
  const AnnotatedLog log1_{ makeLog(1, R"( { "foo": "xxx", "bar": 42 } )") };
  const AnnotatedLog log2_{ makeLog(2, R"( { "foo": "yyy", "bar": 44 } )") };
  const AnnotatedLog log3_{ makeLog(3, R"( { "foo": "xxx", "bar": 46 } )") };
  const AnnotatedLog log4_{ makeLog(4, R"( { "xxx": "xxx", "bar": 48 } )") };
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
  const auto log = LogATE::AnnotatedLog{ makeLog(1, R"( { "foo": "xxx", "bar": { "foo": "yyy" } } )") };
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


TEST_CASE_FIXTURE(Fixture, "trimmed fields setting and propagation")
{
  SUBCASE("root node has trimmed fileds set")
  {
    const auto tf = explode_.trimFields();
    REQUIRE( tf.size() == 1 );
    CHECK( tf[0] == defaultPath_ );
  }
  SUBCASE("children derive trimmed nodes")
  {
    explode_.insert(log1_);
    const auto children = explode_.children();
    REQUIRE( children.size() == 2 );
    for(auto& c: children)
      CHECK( c->trimFields() == explode_.trimFields() );
  }
  SUBCASE("children derive trimmed nodes when adding to a new parent")
  {
    const auto tf = Node::TrimFields{ Path::parse("xxx") };
    AcceptAll root{ workers_, Node::Name{"root"}, tf };
    const auto explode = root.add( But::makeUniqueNN<Explode>( workers_, Explode::Name{"foo"}, defaultPath_ ) );

    {
      const auto out = explode->trimFields();
      REQUIRE( out.size() == 2 );
      CHECK( out[0] == defaultPath_ );
      CHECK( out[1] == tf.front() );
    }

    {
      const auto children = explode->children();
      REQUIRE( children.size() == 1 );
      for(auto& c: children)
        CHECK( c->trimFields() == explode->trimFields() );
    }
  }
}


TEST_CASE_FIXTURE(Fixture, "insert() always adds a node")
{
  for(auto& log: {log1_, log2_, log3_, log4_})
    CHECK( explode_.insert(log) == true );
  auto sum = 0u;
  for(auto& n: extractLogs())
    sum += n.second.size();
  CHECK( sum == 4 );
}

}
}
