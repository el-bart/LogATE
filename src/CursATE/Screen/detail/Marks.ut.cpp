#include <doctest/doctest.h>
#include "CursATE/Screen/detail/Marks.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/TestHelpers.ut.hpp"

using CursATE::Screen::detail::Marks;
using LogATE::makeKey;

namespace
{
TEST_SUITE("CursATE::Screen::detail::Marks")
{

struct Fixture
{
  auto makeNode() const
  {
    using LogATE::Tree::Filter::AcceptAll;
    return But::makeSharedNN<AcceptAll>(workers_, AcceptAll::Name{"united states of whatever"});
  }

  const LogATE::Utils::WorkerThreadsShPtr workers_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>() };
  Marks m_;
};


TEST_CASE_FIXTURE(Fixture, "empy by default")
{
  CHECK( m_.size() == 0u );
  CHECK_FALSE( m_.find('x') );
}


TEST_CASE_FIXTURE(Fixture, "caching and getting")
{
  const auto key1 = makeKey();
  const auto node1 = makeNode();
  m_.insert( 'a', Marks::Entry{key1, node1} );
  {
    const auto e = m_.find('a');
    REQUIRE(static_cast<bool>(e) == true);
    CHECK( e->key_ == key1 );
    const auto node = e->node_.lock();
    CHECK( node.get() == node1.get() );
  }

  {
    const auto e = m_.find('b');
    REQUIRE(static_cast<bool>(e) == false);
  }

  const auto key2 = makeKey();
  const auto node2 = makeNode();
  m_.insert( 'b', Marks::Entry{key2, node2} );
  {
    const auto e = m_.find('a');
    REQUIRE(static_cast<bool>(e) == true);
    CHECK( e->key_ == key1 );
    const auto node = e->node_.lock();
    CHECK( node.get() == node1.get() );
  }
  {
    const auto e = m_.find('b');
    REQUIRE( static_cast<bool>(e) == true);
    CHECK( e->key_ == key2 );
    const auto node = e->node_.lock();
    CHECK( node.get() == node2.get() );
  }
}


TEST_CASE_FIXTURE(Fixture, "insertiuon for existing key overrides")
{
  const auto key1 = makeKey();
  const auto node1 = makeNode();
  m_.insert( 'a', Marks::Entry{key1, node1} );

  const auto key2 = makeKey();
  const auto node2 = makeNode();
  m_.insert( 'a', Marks::Entry{key2, node2} );

  const auto e = m_.find('a');
  REQUIRE(static_cast<bool>(e) == true);
  CHECK( e->key_ == key2 );
  const auto node = e->node_.lock();
  CHECK( node.get() == node2.get() );
}


TEST_CASE_FIXTURE(Fixture, "getting invalidated entry")
{
  auto key1 = makeKey();
  auto node1 = makeNode().underlyingPointer();
  m_.insert( 'a', Marks::Entry{key1, node1} );
  node1.reset();

  const auto e = m_.find('a');
  REQUIRE(static_cast<bool>(e) == true);
  CHECK( e->key_ == key1 );
  const auto node = e->node_.lock();
  CHECK( node.get() == nullptr );
}


TEST_CASE_FIXTURE(Fixture, "prunning removes invalidated entries")
{
  auto key1 = makeKey();
  auto node1 = makeNode().underlyingPointer();
  m_.insert( 'a', Marks::Entry{key1, node1} );

  auto key2 = makeKey();
  auto node2 = makeNode();
  m_.insert( 'x', Marks::Entry{key2, node2} );

  CHECK( m_.size() == 2u );
  node1.reset();
  CHECK( m_.size() == 2u );
  m_.prune();
  CHECK( m_.size() == 1u );

  {
    const auto e = m_.find('a');
    CHECK(static_cast<bool>(e) == false);
  }
  {
    const auto e = m_.find('x');
    REQUIRE(static_cast<bool>(e) == true);
    CHECK( e->key_ == key2 );
    const auto node = e->node_.lock();
    CHECK( node.get() == node2.get() );
  }
}

}
}
