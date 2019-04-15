#include <doctest/doctest.h>
#include "LogATE/Tree/Path.hpp"
#include "LogATE/TestPrints.ut.hpp"

using LogATE::Tree::Path;

namespace
{

TEST_SUITE("Tree::Path")
{

TEST_CASE("isRoot() works")
{
  CHECK( Path{{}}.root() == true );
  CHECK( Path{{"."}}.root() == true );
  CHECK( Path{{".", "foo", "bar"}}.root() == true );
  CHECK( Path{{"oops", ".", "foo", "bar"}}.root() == false );
  CHECK( Path{{"foo", "bar"}}.root() == false );
}

TEST_CASE("parsing from string")
{
  CHECK( Path{{"."}} == Path::parse("") );
  CHECK( Path{{"."}} == Path::parse(".") );
  CHECK( Path{{"foo"}} == Path::parse("foo") );
  CHECK( Path{{"foo", "bar"}} == Path::parse("foo.bar") );
  CHECK( Path{{".", "foo", "bar"}} == Path::parse(".foo.bar") );
  CHECK( Path{{"space is ok", "bar"}} == Path::parse("space is ok.bar") );

  CHECK_THROWS_AS( Path::parse(".."),        Path::EmptyNodeInPath );
  CHECK_THROWS_AS( Path::parse(".foo.bar."), Path::EmptyNodeInPath );
  CHECK_THROWS_AS( Path::parse("foo..bar"),  Path::EmptyNodeInPath );
  CHECK_THROWS_AS( Path::parse("foo...bar"), Path::EmptyNodeInPath );
  CHECK_THROWS_AS( Path::parse("..foo.bar"), Path::EmptyNodeInPath );
  CHECK_THROWS_AS( Path::parse("foo.bar.."), Path::EmptyNodeInPath );
}

}

}
