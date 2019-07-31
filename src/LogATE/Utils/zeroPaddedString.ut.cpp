#include <doctest/doctest.h>
#include "LogATE/Utils/zeroPaddedString.hpp"
#include <limits>

using LogATE::Utils::zeroPaddedString;

namespace
{
TEST_SUITE("Tree::Utils::zeroPaddedString")
{

TEST_CASE("conversion to string")
{
  constexpr auto expectedLen = std::numeric_limits<uint64_t>::digits10 + 1u;
  CHECK( zeroPaddedString( 0).size() == expectedLen );
  CHECK( zeroPaddedString( 0) == "00000000000000000000" );
  CHECK( zeroPaddedString( 1) == "00000000000000000001" );
  CHECK( zeroPaddedString(10) == "00000000000000000010" );
  CHECK( zeroPaddedString(123456789ul) == "00000000000123456789" );
  CHECK( zeroPaddedString( std::numeric_limits<uint64_t>::max() ).size() == expectedLen );
}

}
}
