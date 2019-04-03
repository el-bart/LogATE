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

  Explode explode_{ Explode::Name{"foo"}, Path{{".", "foo"}} };
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
}

TEST_CASE_FIXTURE(Fixture, "new value creates new destination")
{
}

TEST_CASE_FIXTURE(Fixture, "logs w/o required fieled go to default location")
{
}

TEST_CASE_FIXTURE(Fixture, "field can be specified with a relative path")
{
}

TEST_CASE_FIXTURE(Fixture, "ambigous relative path adds logs to multiple cathegories")
{
}

TEST_CASE_FIXTURE(Fixture, "explicit addition of child fails")
{
  using LogATE::Tree::Filter::AcceptAll;
  CHECK_THROWS_AS( explode_.add( But::makeUniqueNN<AcceptAll>(name("xxx")) ), Explode::ExplicitNodeAddNotSupported );
}

}
}
