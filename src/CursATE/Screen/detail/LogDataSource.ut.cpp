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
  const auto n = (*log.log_)["foo"].get<int>();
  return std::to_string(n);
}

struct Fixture
{
  auto makeLog(const int sn, std::string const& str) const
  {
    auto log = LogATE::json2log(str);
    log.sn_.value_ = sn;
    return log;
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
  node_->insert( makeLog(1, R"({ "foo": 41 })") );
  node_->insert( makeLog(2, R"({ "foo": 42 })") );
  node_->insert( makeLog(4, R"({ "foo": 44 })") );
  node_->insert( makeLog(6, R"({ "foo": 46 })") );
  node_->insert( makeLog(8, R"({ "foo": 48 })") );

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

}
}
