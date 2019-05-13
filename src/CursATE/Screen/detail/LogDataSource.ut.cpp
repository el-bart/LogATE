#include <doctest/doctest.h>
#include "CursATE/Screen/detail/LogDataSource.hpp"
#include "CursATE/Curses/TestPrints.ut.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include <sstream>

using CursATE::Curses::DataSource;

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
    return LogATE::AnnotatedLog{ LogATE::Log{ LogATE::SequenceNumber{sn}, str } };
  }

  LogATE::Utils::WorkerThreadsShPtr workers_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>() };
  LogATE::Tree::NodeShPtr node_{ But::makeSharedNN<LogATE::Tree::Filter::AcceptAll>(workers_, LogATE::Tree::Node::Name{"foo-bar"}) };
  CursATE::Screen::detail::LogDataSource lds_{node_, log2str};
};


TEST_CASE_FIXTURE(Fixture, "empty logs do not return any data")
{
  CHECK( lds_.size() == 0 );
  CHECK( not lds_.first() );
  CHECK( not lds_.last() );
  CHECK( lds_.get(1000, DataSource::Id{42}, 1000).empty() );
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
    CHECK(opt->value_ == 1);
  }
  {
    const auto opt = lds_.last();
    REQUIRE(opt);
    CHECK(opt->value_ == 8);
  }
  {
    auto out = lds_.get(1000, DataSource::Id{42}, 1000);
    CHECK( out.size() == 5 );
    CHECK( out[DataSource::Id{1}] == "41" );
    CHECK( out[DataSource::Id{2}] == "42" );
    CHECK( out[DataSource::Id{4}] == "44" );
    CHECK( out[DataSource::Id{6}] == "46" );
    CHECK( out[DataSource::Id{8}] == "48" );
  }
}


auto expectId(const But::Optional<DataSource::Id> id)
{
  REQUIRE(id);
  return id->value_;
}

TEST_CASE_FIXTURE(Fixture, "finding nearest log")
{
  using Id = DataSource::Id;
  CHECK( not lds_.nearestTo( Id{42} ) );

  node_->insert( makeAnnotatedLog(1, R"({ "foo": 41 })") );
  CHECK( expectId( lds_.nearestTo( Id{0} ) ) == 1 );
  CHECK( expectId( lds_.nearestTo( Id{1} ) ) == 1 );
  CHECK( expectId( lds_.nearestTo( Id{42} ) ) == 1 );

  node_->insert( makeAnnotatedLog(2, R"({ "foo": 42 })") );
  node_->insert( makeAnnotatedLog(4, R"({ "foo": 44 })") );
  node_->insert( makeAnnotatedLog(6, R"({ "foo": 46 })") );
  node_->insert( makeAnnotatedLog(18, R"({ "foo": 48 })") );

  SUBCASE("exact matches")
  {
    CHECK( expectId( lds_.nearestTo( Id{1} ) ) == 1 );
    CHECK( expectId( lds_.nearestTo( Id{2} ) ) == 2 );
  }
  SUBCASE("non-exact matches")
  {
    CHECK( expectId( lds_.nearestTo( Id{0} ) ) == 1 );
    CHECK( expectId( lds_.nearestTo( Id{3} ) ) == 4 );
    CHECK( expectId( lds_.nearestTo( Id{5} ) ) == 6 );
    CHECK( expectId( lds_.nearestTo( Id{7} ) ) == 6 );
    CHECK( expectId( lds_.nearestTo( Id{10} ) ) == 6 );
    CHECK( expectId( lds_.nearestTo( Id{16} ) ) == 18 );
    CHECK( expectId( lds_.nearestTo( Id{666} ) ) == 18 );
  }
}


TEST_CASE_FIXTURE(Fixture, "finding index of an element")
{
  using Id = DataSource::Id;

  CHECK( lds_.index( Id{1} ) == 0 );
  CHECK( lds_.index( Id{2} ) == 0 );

  node_->insert( makeAnnotatedLog(1, R"({ "foo": 41 })") );
  CHECK( lds_.index( Id{1} ) == 0 );
  CHECK( lds_.index( Id{2} ) == 0 );

  node_->insert( makeAnnotatedLog(2, R"({ "foo": 42 })") );
  node_->insert( makeAnnotatedLog(4, R"({ "foo": 44 })") );
  node_->insert( makeAnnotatedLog(6, R"({ "foo": 46 })") );
  node_->insert( makeAnnotatedLog(8, R"({ "foo": 48 })") );
  CHECK( lds_.index( Id{2} ) == 1 );
  CHECK( lds_.index( Id{3} ) == 0 );
  CHECK( lds_.index( Id{4} ) == 2 );
  CHECK( lds_.index( Id{5} ) == 0 );
  CHECK( lds_.index( Id{6} ) == 3 );
  CHECK( lds_.index( Id{7} ) == 0 );
  CHECK( lds_.index( Id{8} ) == 4 );
  CHECK( lds_.index( Id{9} ) == 0 );
}

}
}
