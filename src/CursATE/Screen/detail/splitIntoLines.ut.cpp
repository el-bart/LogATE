#include <doctest/doctest.h>
#include "CursATE/Screen/detail/splitIntoLines.hpp"

using CursATE::Screen::detail::splitIntoLines;
using namespace std::literals::string_literals;

namespace
{
TEST_SUITE("CursATE::Screen::detail::splitIntoLines")
{


TEST_CASE("empty string")
{
  CHECK( splitIntoLines("", 100).empty() );
}


TEST_CASE("single line")
{
  const auto out = splitIntoLines("foo bar", 100);
  REQUIRE( out.size() == 1u );
  CHECK( out[0] == "foo bar" );
}


TEST_CASE("wrapping lines")
{
  SUBCASE("exactly the column size does not break line")
  {
    const auto out = splitIntoLines("foobar", 6);
    REQUIRE( out.size() == 1u );
    CHECK( out[0] == "foobar" );
  }
  SUBCASE("over line lenght break the line")
  {
    const auto out = splitIntoLines("foobarX", 6);
    REQUIRE( out.size() == 2u );
    CHECK( out[0] == "foobar" );
    CHECK( out[1] == "X" );
  }
  SUBCASE("round multi-lines")
  {
    const auto out = splitIntoLines("foobar012345", 6);
    REQUIRE( out.size() == 2u );
    CHECK( out[0] == "foobar" );
    CHECK( out[1] == "012345" );
  }
  SUBCASE("non-round multi-lines")
  {
    const auto out = splitIntoLines("foobar012345xxx", 6);
    REQUIRE( out.size() == 3u );
    CHECK( out[0] == "foobar" );
    CHECK( out[1] == "012345" );
    CHECK( out[2] == "xxx" );
  }
  SUBCASE("escape sequence serve as split point")
  {
    const auto out = splitIntoLines("foo\\bar", 6);
    REQUIRE( out.size() == 2u );
    CHECK( out[0] == "foo\\b" );
    CHECK( out[1] == "ar" );
  }
  SUBCASE("when escaped EOL is in the line break place, it is moved to a new line")
  {
    const auto out = splitIntoLines("foo\\nbar", 4);
    REQUIRE( out.size() == 3u );
    CHECK( out[0] == "foo" );
    CHECK( out[1] == "\\n" );
    CHECK( out[2] == "bar" );
  }
  SUBCASE("when escaped non-EOL is in the line break place, it is moved to a new line")
  {
    const auto out = splitIntoLines("foo\\tx", 4);
    REQUIRE( out.size() == 2u );
    CHECK( out[0] == "foo" );
    CHECK( out[1] == "\\tx" );
  }
  SUBCASE("when escaped non-printable char is in the line break place, it is moved to a new line (whole sequence)")
  {
    const auto out = splitIntoLines("foo\\x42bar", 4);
    REQUIRE( out.size() == 3u );
    CHECK( out[0] == "foo" );
    CHECK( out[1] == "\\x42" );
    CHECK( out[2] == "bar" );
  }
  SUBCASE("when escaped non-printable char is in the line break place, it is moved to a new line (sequence is part of it)")
  {
    const auto out = splitIntoLines("foo\\x42bar", 5);
    REQUIRE( out.size() == 3u );
    CHECK( out[0] == "foo" );
    CHECK( out[1] == "\\x42" );
    CHECK( out[2] == "bar" );
  }
  SUBCASE("columns are always at least 4 characters wide")
  {
    const auto out = splitIntoLines("foo\\x42bar", 2);
    REQUIRE( out.size() == 3u );
    CHECK( out[0] == "foo" );
    CHECK( out[1] == "\\x42" );
    CHECK( out[2] == "bar" );
  }
  SUBCASE("EOL at the end of last line does not break line")
  {
    const auto out = splitIntoLines("foobar012345x\\nok\\n", 6);
    REQUIRE( out.size() == 4u );
    CHECK( out[0] == "foobar" );
    CHECK( out[1] == "012345" );
    CHECK( out[2] == "x\\n" );
    CHECK( out[3] == "ok\\n" );
  }
}


TEST_CASE("wrapping lines around words, when possible")
{
  SUBCASE("wrap aroud space")
  {
    const auto out = splitIntoLines("foo bar", 6);
    REQUIRE( out.size() == 2u );
    CHECK( out[0] == "foo " );
    CHECK( out[1] == "bar" );
  }
  SUBCASE("wrap aroud tab")
  {
    const auto out = splitIntoLines("foo\\tbar", 6);
    REQUIRE( out.size() == 2u );
    CHECK( out[0] == "foo\\t" );
    CHECK( out[1] == "bar" );
  }
  SUBCASE("wrap aroud CR")
  {
    const auto out = splitIntoLines("foo\\nbar", 6);
    REQUIRE( out.size() == 2u );
    CHECK( out[0] == "foo\\n" );
    CHECK( out[1] == "bar" );
  }
  SUBCASE("wrap aroud LF")
  {
    const auto out = splitIntoLines("foo\\rbar", 6);
    REQUIRE( out.size() == 2u );
    CHECK( out[0] == "foo\\r" );
    CHECK( out[1] == "bar" );
  }
  SUBCASE("wrap aroud CR+FL is considered single new line")
  {
    const auto out = splitIntoLines("foo\\n\\r\\n\\rbar", 7);
    REQUIRE( out.size() == 3u );
    CHECK( out[0] == "foo\\n\\r" );
    CHECK( out[1] == "\\n\\r" );
    CHECK( out[2] == "bar" );
  }
  SUBCASE("wrap aroud FL+CR is considered single new line")
  {
    const auto out = splitIntoLines("foo\\r\\n\\r\\nbar", 7);
    REQUIRE( out.size() == 3u );
    CHECK( out[0] == "foo\\r\\n" );
    CHECK( out[1] == "\\r\\n" );
    CHECK( out[2] == "bar" );
  }
  SUBCASE("wrapping multiple CRs are considered separate lines")
  {
    const auto out = splitIntoLines("foo\\n\\nbar", 7);
    REQUIRE( out.size() == 3u );
    CHECK( out[0] == "foo\\n" );
    CHECK( out[1] == "\\n" );
    CHECK( out[2] == "bar" );
  }
  SUBCASE("wrapping multiple LFs are considered separate lines")
  {
    const auto out = splitIntoLines("foo\\r\\rbar", 7);
    REQUIRE( out.size() == 3u );
    CHECK( out[0] == "foo\\r" );
    CHECK( out[1] == "\\r" );
    CHECK( out[2] == "bar" );
  }
  SUBCASE("any whitespace is a good split point")
  {
    const auto out = splitIntoLines("a     z", 6);
    REQUIRE( out.size() == 2u );
    CHECK( out[0] == "a     " );
    CHECK( out[1] == "z" );
  }
  SUBCASE("more white spaces than column len")
  {
    const auto out = splitIntoLines("a            z", 6);
    REQUIRE( out.size() == 3u );
    CHECK( out[0] == "a     " );
    CHECK( out[1] == "      " );
    CHECK( out[2] == " z" );
  }
}

// TODO: pseudo-random long text + concatenation test

}
}
