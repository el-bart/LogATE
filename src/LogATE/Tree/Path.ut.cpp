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
  CHECK( Path::parse("").absolute() == false );
  CHECK( Path::parse(".").absolute() == true );
  CHECK( Path::parse(".foo.bar").absolute() == true );
  CHECK( Path::parse("oops.foo.bar").absolute() == false );
  CHECK( Path::parse("foo.bar").absolute() == false );
  CHECK( Path::parse("foo[].bar[42]").absolute() == false );
}


Path::Data refPath(std::vector<std::string> in)
{
  Path::Data d;
  for(auto& e: in)
    d.emplace_back( Path::Entry{ std::move(e) } );
  return d;
}


TEST_CASE("parsing from string")
{
  CHECK( refPath({}) == Path::parse("").data() );
  CHECK( refPath({"."}) == Path::parse(".").data() );
  CHECK( refPath({"."}) == Path::parse(".").data() );
  CHECK( refPath({"foo"}) == Path::parse("foo").data() );
  CHECK( refPath({"foo", "bar"}) == Path::parse("foo.bar").data() );
  CHECK( refPath({"foo", "bar"}) == Path::parse(".foo.bar").data() );   // the leading dot is just for marking path as absolute
  CHECK( refPath({"space is ok", "bar"}) == Path::parse("space is ok.bar").data() );

  CHECK_THROWS_AS( Path::parse(".[42]"),     Path::EmptyNodeInPath );
  CHECK_THROWS_AS( Path::parse(".."),        Path::EmptyNodeInPath );
  CHECK_THROWS_AS( Path::parse(".foo.bar."), Path::EmptyNodeInPath );
  CHECK_THROWS_AS( Path::parse("foo..bar"),  Path::EmptyNodeInPath );
  CHECK_THROWS_AS( Path::parse("foo...bar"), Path::EmptyNodeInPath );
  CHECK_THROWS_AS( Path::parse("..foo.bar"), Path::EmptyNodeInPath );
  CHECK_THROWS_AS( Path::parse("foo.bar.."), Path::EmptyNodeInPath );
  CHECK_THROWS_AS( Path::parse("foo[.bar"),  Path::EmptyNodeInPath );
  CHECK_THROWS_AS( Path::parse("foo].bar"),  Path::EmptyNodeInPath );
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

// TODO: arrays handling - "[42]" should also be fine, in case first node is addressed, but 'foo.[42].bar" should be an error, since name is missing in the following ones

}
}
