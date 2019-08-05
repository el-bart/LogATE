#include <doctest/doctest.h>
#include "CursATE/Screen/detail/LogIdIndexCache.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/TestHelpers.ut.hpp"
#include <sstream>

using CursATE::Curses::DataSource;
using CursATE::Screen::detail::key2id;
using LogATE::makeKey;

namespace
{
TEST_SUITE("CursATE::Screen::detail::LogIdIndexCache")
{

struct Fixture
{
  LogATE::Utils::WorkerThreadsShPtr workers_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>() };
  LogATE::Tree::NodeShPtr node_{ But::makeSharedNN<LogATE::Tree::Filter::AcceptAll>(workers_, LogATE::Tree::Node::Name{"foo-bar"}) };
  const CursATE::Screen::detail::LogIdIndexCache liic_{node_};
};


TEST_CASE_FIXTURE(Fixture, "empty node retunrs 0")
{
}


TEST_CASE_FIXTURE(Fixture, "finding elements in node")
{
}


TEST_CASE_FIXTURE(Fixture, "finding edge elements")
{
}


TEST_CASE_FIXTURE(Fixture, "auto-cleanup of outdated entries")
{
}

}
}
