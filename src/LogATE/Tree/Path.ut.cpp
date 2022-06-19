#include <doctest/doctest.h>
#include "LogATE/Tree/Path.hpp"
#include "LogATE/TestPrints.ut.hpp"

using LogATE::Tree::Path;

namespace
{
TEST_SUITE("Tree::Path")
{

TEST_CASE("absolute path is detected correctly")
{
  CHECK( Path::parse("").isAbsolute() == false );
  CHECK( Path::parse(".").isAbsolute() == true );
  CHECK( Path::parse(".foo.bar").isAbsolute() == true );
  CHECK( Path::parse("oops.foo.bar").isAbsolute() == false );
  CHECK( Path::parse("foo[].bar[42]").isAbsolute() == false );
}


TEST_CASE("uniqueness of path is detected correctly")
{
  SUBCASE("non-absolute paths are not-unique")
  {
    CHECK( Path::parse("").isUnique() == false );
    CHECK( Path::parse("oops.foo.bar").isUnique() == false );
    CHECK( Path::parse("foo.bar").isUnique() == false );
    CHECK( Path::parse("foo[].bar[42]").isUnique() == false );
  }
  SUBCASE("absolute path w/o wildcards are unique")
  {
    CHECK( Path::parse(".").isUnique() == true );
    CHECK( Path::parse(".foo.bar").isUnique() == true );
    CHECK( Path::parse(".foo[42].bar").isUnique() == true );
    CHECK( Path::parse(".foo[42].bar[13]").isUnique() == true );
  }
  SUBCASE("absolute path with wildcards are not unique")
  {
    CHECK( Path::parse(".foo[].bar").isUnique() == false );
    CHECK( Path::parse(".foo.bar[]").isUnique() == false );
  }
}


TEST_CASE("parsing from string")
{
  CHECK( Path{{}} == Path::parse("") );
  CHECK( Path{{"."}} == Path::parse(".") );
  CHECK( Path{{"foo"}} == Path::parse("foo") );
  CHECK( Path{{"foo", "bar"}} == Path::parse("foo.bar") );
  CHECK( Path{{".", "foo", "bar"}} == Path::parse(".foo.bar") );
  CHECK( Path{{"space is ok", "bar"}} == Path::parse("space is ok.bar") );

  CHECK_THROWS_AS( Path::parse(".[]"),       Path::Entry::InvalidArray );
  CHECK_THROWS_AS( Path::parse(".[42]"),     Path::Entry::InvalidArray );
  CHECK_THROWS_AS( Path::parse(".."),        Path::Entry::EmptyNode );
  CHECK_THROWS_AS( Path::parse(".foo.bar."), Path::Entry::EmptyNode );
  CHECK_THROWS_AS( Path::parse("foo..bar"),  Path::Entry::EmptyNode );
  CHECK_THROWS_AS( Path::parse("foo...bar"), Path::Entry::EmptyNode );
  CHECK_THROWS_AS( Path::parse("..foo.bar"), Path::Entry::EmptyNode );
  CHECK_THROWS_AS( Path::parse("foo.bar.."), Path::Entry::EmptyNode );
  CHECK_THROWS_AS( Path::parse("foo[.bar"),  Path::Entry::InvalidArray );
  CHECK_THROWS_AS( Path::parse("foo].bar"),  Path::Entry::InvalidArray );
}


TEST_CASE("converting to string")
{
  CHECK( Path::parse("").str() == "" );
  CHECK( Path::parse(".").str() == "." );
  CHECK( Path::parse("foo").str() == "foo" );
  CHECK( Path::parse("foo.bar").str() == "foo.bar" );
  CHECK( Path::parse(".foo.bar").str() == ".foo.bar" );
  CHECK( Path::parse("space is ok.bar").str() == "space is ok.bar" );
}


TEST_CASE("building from vector of strings")
{
  CHECK( Path::build({}) == Path::parse("") );
  CHECK( Path::build({"."}) == Path::parse(".") );
  CHECK( Path::build({"foo"}) == Path::parse("foo") );
  CHECK( Path::build({".", "foo"}) == Path::parse(".foo") );
  CHECK( Path::build({"foo", "bar"}) == Path::parse("foo.bar") );
}


TEST_CASE("creating empty path")
{
  const auto empty = Path{};
  CHECK( empty.empty() == true );
  CHECK( empty.isAbsolute() == false );
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

}
}
