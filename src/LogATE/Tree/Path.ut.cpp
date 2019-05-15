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
  CHECK( Path{{}}.absolute() == false );
  CHECK( Path{{"."}}.absolute() == true );
  CHECK( Path{{".", "foo", "bar"}}.absolute() == true );
  CHECK( Path{{"oops", ".", "foo", "bar"}}.absolute() == false );
  CHECK( Path{{"foo", "bar"}}.absolute() == false );
}


TEST_CASE("parsing from string")
{
  CHECK( Path{} == Path::parse("") );
  CHECK( Path{{}} == Path::parse("") );
  CHECK( Path{{"."}} == Path::parse(".") );
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


TEST_CASE("converting to string")
{
  CHECK( Path{}.str() == "" );
  CHECK( Path{{}}.str() == "" );
  CHECK( Path{{"."}}.str() == "." );
  CHECK( Path{{"foo"}}.str() == "foo" );
  CHECK( Path{{"foo", "bar"}}.str() == "foo.bar" );
  CHECK( Path{{".", "foo", "bar"}}.str() == ".foo.bar" );
  CHECK( Path{{"space is ok", "bar"}}.str() == "space is ok.bar" );
}


TEST_CASE("creating empty path")
{
  const auto empty = Path{};
  CHECK( empty.empty() == true );
  CHECK( empty.absolute() == false );
  CHECK( empty.begin() == empty.end() );
  CHECK( empty.str() == "" );
}

}
}
