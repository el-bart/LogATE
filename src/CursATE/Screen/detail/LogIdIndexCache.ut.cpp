#include <doctest/doctest.h>
#include "CursATE/Screen/detail/LogIdIndexCache.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/TestHelpers.ut.hpp"
#include <sstream>

using LogATE::makeKey;

namespace
{
TEST_SUITE("CursATE::Screen::detail::LogIdIndexCache")
{

struct Fixture
{
  auto makeLog(unsigned sn, std::string key) const { return LogATE::Log{ makeKey(sn, std::move(key)), "{}" }; }

  void fillWithData()
  {
    node_->insert( LogATE::AnnotatedLog{ makeLog(1, "bar") } );
    node_->insert( LogATE::AnnotatedLog{ makeLog(4, "bar") } );
    node_->insert( LogATE::AnnotatedLog{ makeLog(0, "foo") } );
    node_->insert( LogATE::AnnotatedLog{ makeLog(8, "kamboom!") } );
    node_->insert( LogATE::AnnotatedLog{ makeLog(5, "kszy") } );
    node_->insert( LogATE::AnnotatedLog{ makeLog(3, "narf") } );
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
  node_->insert( LogATE::AnnotatedLog{ makeLog(0, "foo") } );
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

// TODO: pseudo-random search in a big data set

}
}
