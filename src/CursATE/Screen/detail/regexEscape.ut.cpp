#include <doctest/doctest.h>
#include "CursATE/Screen/detail/regexEscape.hpp"

using CursATE::Screen::detail::regexEscape;

namespace
{
TEST_SUITE("CursATE::Screen::detail::regexEscape")
{


TEST_CASE("direct output")
{
  CHECK( regexEscape("") == "" );
  CHECK( regexEscape("abc") == "abc" );
  CHECK( regexEscape("a/b#c") == "a/b#c" );
  CHECK( regexEscape("a b\tc\n") == "a b\tc\n" );
}


TEST_CASE("escaping")
{
  CHECK( regexEscape(R"([])") == R"(\[\])" );
  CHECK( regexEscape(R"({})") == R"(\{\})" );
  CHECK( regexEscape(R"(\)") == R"(\\)" );
  CHECK( regexEscape(R"(*+?)") == R"(\*\+\?)" );
  CHECK( regexEscape(R"(.)") == R"(\.)" );
  CHECK( regexEscape(R"(|)") == R"(\|)" );
  CHECK( regexEscape(R"(^$)") == R"(\^\$)" );
}

}
}
