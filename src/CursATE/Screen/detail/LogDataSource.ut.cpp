#include <doctest/doctest.h>
#include "CursATE/Screen/detail/LogDataSource.hpp"
#include "CursATE/Screen/detail/id2key.hpp"
#include "CursATE/Curses/TestPrints.ut.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/TestHelpers.ut.hpp"
#include <sstream>

using CursATE::Curses::DataSource;
using CursATE::Screen::detail::key2id;
using Id = DataSource::Id;

namespace
{
TEST_SUITE("CursATE::Screen::detail::LogDataSource")
{

std::string log2str(LogATE::Log const& log)
{
  const auto n = log.json()["foo"].get<int>();
  return std::to_string(n);
}

struct Fixture
{
  auto makeAnnotatedLog(const unsigned sn, std::string const& str) const
  {
    return LogATE::AnnotatedLog{ LogATE::makeLog(sn, str) };
  }

  auto sn2id(const unsigned sn) const { return key2id( LogATE::makeKey(sn) ); }

  LogATE::Utils::WorkerThreadsShPtr workers_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>() };
  LogATE::Tree::NodeShPtr node_{ But::makeSharedNN<LogATE::Tree::Filter::AcceptAll>(workers_, LogATE::Tree::Node::Name{"foo-bar"}) };
  CursATE::Screen::detail::LogDataSource lds_{node_, log2str};
};


TEST_CASE_FIXTURE(Fixture, "empty logs do not return any data")
{
  CHECK( lds_.size() == 0 );
  CHECK( not lds_.first() );
  CHECK( not lds_.last() );
  CHECK( lds_.get(1000, sn2id(42), 1000).empty() );
}


TEST_CASE_FIXTURE(Fixture, "non-empty logs")
{
  node_->insert( makeAnnotatedLog(1, R"({ "foo": 41 })") );
  node_->insert( makeAnnotatedLog(2, R"({ "foo": 42 })") );
  node_->insert( makeAnnotatedLog(4, R"({ "foo": 44 })") );
  node_->insert( makeAnnotatedLog(6, R"({ "foo": 46 })") );
  node_->insert( makeAnnotatedLog(8, R"({ "foo": 48 })") );

  CHECK( lds_.size() == 5 );
  {
    const auto opt = lds_.first();
    REQUIRE(opt);
    CHECK(*opt == sn2id(1));
  }
  {
    const auto opt = lds_.last();
    REQUIRE(opt);
    CHECK(*opt == sn2id(8));
  }
  {
    auto out = lds_.get(1000, sn2id(42), 1000);
    CHECK( out.size() == 5 );
    CHECK( out[sn2id(1)] == "41" );
    CHECK( out[sn2id(2)] == "42" );
    CHECK( out[sn2id(4)] == "44" );
    CHECK( out[sn2id(6)] == "46" );
    CHECK( out[sn2id(8)] == "48" );
  }
}


auto expectId(But::Optional<DataSource::Id> id)
{
  REQUIRE(id);
  return *id;
}

TEST_CASE_FIXTURE(Fixture, "finding nearest log")
{
  CHECK( not lds_.nearestTo( sn2id(42) ) );

  node_->insert( makeAnnotatedLog(1, R"({ "foo": 41 })") );
  CHECK( expectId( lds_.nearestTo( sn2id(0) ) ) == sn2id(1) );
  CHECK( expectId( lds_.nearestTo( sn2id(1) ) ) == sn2id(1) );
  CHECK( expectId( lds_.nearestTo( sn2id(42) ) ) == sn2id(1) );

  node_->insert( makeAnnotatedLog(2, R"({ "foo": 42 })") );
  node_->insert( makeAnnotatedLog(4, R"({ "foo": 44 })") );
  node_->insert( makeAnnotatedLog(6, R"({ "foo": 46 })") );
  node_->insert( makeAnnotatedLog(18, R"({ "foo": 48 })") );

  SUBCASE("exact matches")
  {
    CHECK( expectId( lds_.nearestTo( sn2id(1) ) ) == sn2id(1) );
    CHECK( expectId( lds_.nearestTo( sn2id(2) ) ) == sn2id(2) );
  }
  SUBCASE("non-exact matches")
  {
    CHECK( expectId( lds_.nearestTo( sn2id(0) ) ) == sn2id(1) );
    CHECK( expectId( lds_.nearestTo( sn2id(3) ) ) == sn2id(4) );
    CHECK( expectId( lds_.nearestTo( sn2id(5) ) ) == sn2id(6) );
    CHECK( expectId( lds_.nearestTo( sn2id(7) ) ) == sn2id(6) );
    CHECK( expectId( lds_.nearestTo( sn2id(10) ) ) == sn2id(18) );  // NOTE: nearest according to lexical distance
    CHECK( expectId( lds_.nearestTo( sn2id(16) ) ) == sn2id(18) );
    CHECK( expectId( lds_.nearestTo( sn2id(666) ) ) == sn2id(18) );
  }
}


TEST_CASE_FIXTURE(Fixture, "finding index of an element")
{
  CHECK( lds_.index( sn2id(1) ) == 0 );
  CHECK( lds_.index( sn2id(2) ) == 0 );

  node_->insert( makeAnnotatedLog(1, R"({ "foo": 41 })") );
  CHECK( lds_.index( sn2id(1) ) == 0 );
  CHECK( lds_.index( sn2id(2) ) == 0 );

  node_->insert( makeAnnotatedLog(2, R"({ "foo": 42 })") );
  node_->insert( makeAnnotatedLog(4, R"({ "foo": 44 })") );
  node_->insert( makeAnnotatedLog(6, R"({ "foo": 46 })") );
  node_->insert( makeAnnotatedLog(8, R"({ "foo": 48 })") );
  CHECK( lds_.index( sn2id(2) ) == 1 );
  CHECK( lds_.index( sn2id(3) ) == 0 );
  CHECK( lds_.index( sn2id(4) ) == 2 );
  CHECK( lds_.index( sn2id(5) ) == 0 );
  CHECK( lds_.index( sn2id(6) ) == 3 );
  CHECK( lds_.index( sn2id(7) ) == 0 );
  CHECK( lds_.index( sn2id(8) ) == 4 );
  CHECK( lds_.index( sn2id(9) ) == 0 );
}

}
}
