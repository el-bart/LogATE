#include <doctest/doctest.h>
#include "LogATE/Utils/PrintableStringConverter.hpp"
#include <algorithm>

using LogATE::Utils::PrintableStringConverter;
using namespace std::literals::string_literals;


namespace
{
TEST_SUITE("Tree::Utils::PrintableStringConverter")
{

struct Fixture
{
  const PrintableStringConverter psc_{};
};


TEST_CASE_FIXTURE(Fixture, "printable characters")
{
  SUBCASE("lower case")
  {
    const auto str = "abcdefghijklmnopqrstuwvxyz"s;
    CHECK( psc_(str) == str );
  }
  SUBCASE("upper case")
  {
    const auto str = "ABCDEFGHIJKLMNOPQRSTUWVXYZ"s;
    CHECK( psc_(str) == str );
  }
  SUBCASE("digits")
  {
    const auto str = "0123456789"s;
    CHECK( psc_(str) == str );
  }
  SUBCASE("misc")
  {
    const auto str = "`~!@#$%^&*()_+-=|\\[{]}:;\"'<,>.?/ "s;
    CHECK( psc_(str) == str );
  }
}


namespace
{
auto num2str(int n)
{
  std::string s;
  s.push_back( static_cast<unsigned>(n) );
  return s;
}
}

TEST_CASE_FIXTURE(Fixture, "non-printable characters")
{
  CHECK( psc_( num2str(0x01) ) == "\\x01"s );
  CHECK( psc_( num2str(0x02) ) == "\\x02"s );
  CHECK( psc_( num2str(0x10) ) == "\\x10"s );
  CHECK( psc_( num2str(0x80) ) == "\\x80"s );
  CHECK( psc_( num2str(0xFF) ) == "\\xFF"s );
}


TEST_CASE_FIXTURE(Fixture, "non-printable characters with special tags")
{
  CHECK( psc_( num2str(0x00) ) == "\\0"s );
  CHECK( psc_( num2str(0x07) ) == "\\a"s );
  CHECK( psc_( num2str(0x08) ) == "\\b"s );
  CHECK( psc_( num2str(0x09) ) == "\\t"s );
  CHECK( psc_( num2str(0x0a) ) == "\\n"s );
  CHECK( psc_( num2str(0x0b) ) == "\\v"s );
  CHECK( psc_( num2str(0x0c) ) == "\\f"s );
  CHECK( psc_( num2str(0x0d) ) == "\\r"s );
}


TEST_CASE_FIXTURE(Fixture, "all non-printable characters are gone")
{
  std::string s;
  s.resize(256);
  for(auto i=0u; i<256; ++i)
    s[i] = char(i);

  const auto out = psc_(s);
  CHECK( std::all_of( begin(out), end(out), isprint ) );
}

}
}
