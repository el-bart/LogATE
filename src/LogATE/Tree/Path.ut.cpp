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


TEST_CASE("parsing each entry")
{
  SUBCASE("invalid entry name")
  {
    CHECK_THROWS_AS( Path::Entry{""},      Path::Entry::EmptyNode    );
    CHECK_THROWS_AS( Path::Entry{"a.b"},   Path::Entry::InvalidNode  );
    CHECK_THROWS_AS( Path::Entry{"a["},    Path::Entry::InvalidArray );
    CHECK_THROWS_AS( Path::Entry{"[]a"},   Path::Entry::InvalidArray );
    CHECK_THROWS_AS( Path::Entry{"a[b]"},  Path::Entry::InvalidArray );
    CHECK_THROWS_AS( Path::Entry{"a[3"},   Path::Entry::InvalidArray );
    CHECK_THROWS_AS( Path::Entry{"a3]"},   Path::Entry::InvalidArray );
    CHECK_THROWS_AS( Path::Entry{"a[3]b"}, Path::Entry::InvalidArray );
    CHECK_THROWS_AS( Path::Entry{"a[-1]"}, Path::Entry::InvalidArray );
    CHECK_THROWS_AS( Path::Entry{"[]"},    Path::Entry::InvalidArray );
    CHECK_THROWS_AS( Path::Entry{"[a]"},   Path::Entry::InvalidArray );
    CHECK_THROWS_AS( Path::Entry{"[1]"},   Path::Entry::InvalidArray );
    CHECK_THROWS_AS( Path::Entry{"a]1["},  Path::Entry::InvalidArray );
  }
  SUBCASE("object name")
  {
    auto e = Path::Entry{"foo"};
    CHECK( e.name() == "foo" );
    CHECK( e.isArray() == false );
  }
  SUBCASE("regular names")
  {
    CHECK( Path::Entry{"a"}.str() == "a" );
    CHECK( Path::Entry{"abc"}.str() == "abc" );
    CHECK( Path::Entry{"space is ok"}.str() == "space is ok" );
    CHECK( Path::Entry{"misc !@#$%^&*()"}.str() == "misc !@#$%^&*()" );
    CHECK( Path::Entry{"misc !@#$%^&*()"}.str() == "misc !@#$%^&*()" );
  }
  SUBCASE("array element")
  {
    auto e = Path::Entry{"foo[42]"};
    CHECK( e.name() == "foo" );
    REQUIRE( e.isArray() == true );
    REQUIRE( e.hasIndex() == true );
    CHECK( e.index() == 42 );
    CHECK( e.str() == "foo[42]" );

  }
  SUBCASE("array element index with spaces")
  {
    for(auto& str: {"foo[ 42]", "foo[42 ]", "foo[\t42]", "foo[42\t]", "foo[\t 42 \t]"})
    {
      auto e = Path::Entry{str};
      CHECK( e.name() == "foo" );
      REQUIRE( e.isArray() == true );
      REQUIRE( e.hasIndex() == true );
      CHECK( e.index() == 42 );
      CHECK( e.str() == "foo[42]" );
    }

  }
  SUBCASE("any array element")
  {
    for(auto& str: {"foo[]", "foo[ \t  ]"})
    {
      auto e = Path::Entry{str};
      CHECK( e.name() == "foo" );
      REQUIRE( e.isArray() == true );
      REQUIRE( e.hasIndex() == false );
      CHECK( e.str() == "foo[]" );
    }
  }
}

// TODO: arrays handling

}
}
