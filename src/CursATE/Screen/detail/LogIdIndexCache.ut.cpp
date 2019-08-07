#include <doctest/doctest.h>
#include "CursATE/Screen/detail/LogIdIndexCache.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/TestHelpers.ut.hpp"
#include <sstream>
#include <random>

using LogATE::makeKey;

namespace
{
TEST_SUITE("CursATE::Screen::detail::LogIdIndexCache")
{

struct Fixture
{
  static auto makeLog(unsigned sn, std::string key) { return LogATE::Log{ makeKey(sn, std::move(key)), "{}" }; }
  static auto makeAnnotatedLog(unsigned sn, std::string key) { return LogATE::AnnotatedLog( makeLog(sn, std::move(key)) ); }

  void fillWithData()
  {
    node_->insert( makeAnnotatedLog(1, "bar") );
    node_->insert( makeAnnotatedLog(4, "bar") );
    node_->insert( makeAnnotatedLog(0, "foo") );
    node_->insert( makeAnnotatedLog(8, "kamboom!") );
    node_->insert( makeAnnotatedLog(5, "kszy") );
    node_->insert( makeAnnotatedLog(3, "narf") );
  }

  LogATE::Utils::WorkerThreadsShPtr workers_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>() };
  LogATE::Tree::NodeShPtr node_{ But::makeSharedNN<LogATE::Tree::Filter::AcceptAll>(workers_, LogATE::Tree::Node::Name{"foo-bar"}) };
  const CursATE::Screen::detail::LogIdIndexCache liic_{ node_.underlyingPointer() };
};


TEST_CASE_FIXTURE(Fixture, "empty node returns 0")
{
  CHECK( liic_.index( makeKey(666) ) == 0u );
  CHECK( liic_.size() == 0u );
}


TEST_CASE_FIXTURE(Fixture, "non-empty node but w/o given element returns 0")
{
  fillWithData();
  CHECK( liic_.index( makeKey(666) ) == 0u );
  CHECK( liic_.size() == 0u );
}


TEST_CASE_FIXTURE(Fixture, "finding elements in node")
{
  fillWithData();
  CHECK( liic_.size() == 0u );
  CHECK( liic_.index( makeKey(1, "bar") ) == 0 );
  CHECK( liic_.size() == 1u );
  CHECK( liic_.index( makeKey(4, "bar") ) == 1 );
  CHECK( liic_.size() == 2u );
  CHECK( liic_.index( makeKey(0, "foo") ) == 2 );
  CHECK( liic_.size() == 3u );
  CHECK( liic_.index( makeKey(8, "kamboom!") ) == 3 );
  CHECK( liic_.size() == 4u );
  CHECK( liic_.index( makeKey(5, "kszy") ) == 4 );
  CHECK( liic_.size() == 5u );
  CHECK( liic_.index( makeKey(3, "narf") ) == 5 );
  CHECK( liic_.size() == 6u );
  // repeated search
  CHECK( liic_.index( makeKey(8, "kamboom!") ) == 3 );
  CHECK( liic_.size() == 6u );
}


TEST_CASE_FIXTURE(Fixture, "auto-cleanup of outdated entries")
{
  fillWithData();
  node_->pruneUpTo( makeKey(0, "foo") );
  workers_->waitForAll();
  CHECK( liic_.size() == 0u );
  CHECK( liic_.index( makeKey(1, "bar") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( liic_.index( makeKey(4, "bar") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( liic_.index( makeKey(0, "foo") ) == 0 );
  CHECK( liic_.size() == 1u );
  CHECK( liic_.index( makeKey(8, "kamboom!") ) == 1 );
  CHECK( liic_.size() == 2u );
  CHECK( liic_.index( makeKey(5, "kszy") ) == 2 );
  CHECK( liic_.size() == 3u );
  CHECK( liic_.index( makeKey(3, "narf") ) == 3 );
  CHECK( liic_.size() == 4u );

  node_->pruneUpTo( makeKey(666, "xxx") );
  workers_->waitForAll();
  CHECK( liic_.size() == 4u );
  CHECK( liic_.index( makeKey(1, "bar") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( liic_.index( makeKey(4, "bar") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( liic_.index( makeKey(0, "foo") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( liic_.index( makeKey(8, "kamboom!") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( liic_.index( makeKey(5, "kszy") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( liic_.index( makeKey(3, "narf") ) == 0 );
  CHECK( liic_.size() == 0u );
}


TEST_CASE_FIXTURE(Fixture, "test working on 1-element set")
{
  node_->insert( makeAnnotatedLog(0, "foo") );
  CHECK( liic_.size() == 0u );
  CHECK( liic_.index( makeKey(1, "bar") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( liic_.index( makeKey(4, "bar") ) == 0 );
  CHECK( liic_.size() == 0u );
  CHECK( liic_.index( makeKey(0, "foo") ) == 0 );
  CHECK( liic_.size() == 1u );
  CHECK( liic_.index( makeKey(8, "kamboom!") ) == 0 );
  CHECK( liic_.size() == 1u );
  CHECK( liic_.index( makeKey(5, "kszy") ) == 0 );
  CHECK( liic_.size() == 1u );
  CHECK( liic_.index( makeKey(3, "narf") ) == 0 );
  CHECK( liic_.size() == 1u );
}


TEST_CASE_FIXTURE(Fixture, "test random scenarios")
{
  fillWithData();
  CHECK( liic_.size() == 0u );

  SUBCASE("test first elements")
  {
    CHECK( liic_.index( makeKey(1, "bar") ) == 0 );
    CHECK( liic_.size() == 1u );
  }
  SUBCASE("last element")
  {
    CHECK( liic_.index( makeKey(3, "narf") ) == 5 );
    CHECK( liic_.size() == 1u );
  }
  SUBCASE("middle element")
  {
    CHECK( liic_.index( makeKey(8, "kamboom!") ) == 3 );
    CHECK( liic_.size() == 1u );
  }
  SUBCASE("element in between different ones")
  {
    CHECK( liic_.index( makeKey(4, "bar") ) == 1 );
    CHECK( liic_.size() == 1u );
    CHECK( liic_.index( makeKey(5, "kszy") ) == 4 );
    CHECK( liic_.size() == 2u );
    CHECK( liic_.index( makeKey(8, "kamboom!") ) == 3 );
    CHECK( liic_.size() == 3u );
  }
  SUBCASE("search elements in backward order")
  {
    CHECK( liic_.index( makeKey(3, "narf") ) == 5 );
    CHECK( liic_.size() == 1u );
    CHECK( liic_.index( makeKey(5, "kszy") ) == 4 );
    CHECK( liic_.size() == 2u );
    CHECK( liic_.index( makeKey(8, "kamboom!") ) == 3 );
    CHECK( liic_.size() == 3u );
  }
}


auto randomData()
{
  std::vector<LogATE::AnnotatedLog> out;
  const auto alphabet = {0,1,2,3,4,5,6,7,8,9};
  auto counter = 0u;
  for(auto i: alphabet)
    for(auto j: alphabet)
      for(auto k: alphabet)
          out.push_back( Fixture::makeAnnotatedLog( counter++, std::to_string(i) + std::to_string(j) + std::to_string(k) ) );
  return out;
}

auto randomQueries(std::vector<LogATE::AnnotatedLog> const& in, unsigned seed)
{
  std::vector<std::pair<LogATE::Log::Key, size_t>> out;
  out.reserve( in.size() );
  auto pos = 0u;
  for(auto& e: in)
    out.push_back( std::make_pair( e.key(), pos++ ) );
  std::mt19937_64 prng{seed};
  std::shuffle( begin(out), end(out), prng );
  return out;
}

TEST_CASE_FIXTURE(Fixture, "random scenarios - big data sets")
{
  const auto data = randomData();
  for(auto& e: data)
    node_->insert(e);
  CHECK( liic_.size() == 0u );

  SUBCASE("order 1")
  {
    for(auto& q: randomQueries(data, 40+1))
      CHECK( liic_.index(q.first) == q.second );
  }
  SUBCASE("order 2")
  {
    for(auto& q: randomQueries(data, 40+2))
      CHECK( liic_.index(q.first) == q.second );
  }
}

}
}
