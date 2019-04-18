#include <doctest/doctest.h>
#include "LogATE/Tree/FilterFactory.hpp"
#include "LogATE/TestPrints.ut.hpp"

using LogATE::Tree::FilterFactory;
using LogATE::Tree::Node;
using Opts = LogATE::Tree::FilterFactory::Options;

namespace
{
TEST_SUITE("Tree::FilterFactory")
{

struct Fixture
{
  const Node::Name name_{"foo-bar"};
  FilterFactory ff_;
};


TEST_CASE_FIXTURE(Fixture, "constructing AcceptAll filter")
{
  const auto type = FilterFactory::Type{"AcceptAll"};

  SUBCASE("valid")
  {
    CHECK( ff_.build( type, name_, Opts{} )->type().value_ == "AcceptAll" );
  }

  SUBCASE("missing argument")
  {
    // not arguments expected
  }

  SUBCASE("unknown argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{{"foo", "bar"}} ), FilterFactory::UnknownOption );
  }
}

}
}
