#include <doctest/doctest.h>
#include "CursATE/Screen/detail/regexEscape.hpp"
#include "LogATE/Utils/matchesLog.hpp"
#include <regex>

using CursATE::Screen::detail::regexEscape;

namespace
{
TEST_SUITE("CursATE::Screen::detail::regexEscape")
{

#define CHECK_ESCAPING(in, exp) \
  SUBCASE(in) \
  { \
    CHECK( regexEscape(in) == exp ); \
    \
    auto const out = regexEscape(in); \
    try \
    { \
      std::regex const re{"^" + out + "$", LogATE::Utils::g_defaultRegexType}; \
      CHECK( std::regex_search(in, re) ); \
    } \
    catch(std::exception const& ex) \
    { \
      INFO(in); \
      INFO(out); \
      FAIL( ("input string generted invalid RE: " + std::string{ex.what()}) ); \
    } \
  }


TEST_CASE("direct output")
{
  CHECK_ESCAPING( "", "" );
  CHECK_ESCAPING( "abc", "abc" );
  CHECK_ESCAPING( "a/b#c", "a/b#c" );
  CHECK_ESCAPING( "a b\tc\n", "a b\tc\n" );
}


TEST_CASE("escaping")
{
  CHECK_ESCAPING( R"([])", R"(\[\])" );
  CHECK_ESCAPING( R"({})", R"(\{\})" );
  CHECK_ESCAPING( R"(\)", R"(\\)" );
  CHECK_ESCAPING( R"(*+?)", R"(\*\+\?)" );
  CHECK_ESCAPING( R"(.)", R"(\.)" );
  CHECK_ESCAPING( R"(|)", R"(\|)" );
  CHECK_ESCAPING( R"(^$)", R"(\^\$)" );
  CHECK_ESCAPING( R"([foo-bar] xyz...)", R"(\[foo-bar\] xyz\.\.\.)" );
}

}
}
