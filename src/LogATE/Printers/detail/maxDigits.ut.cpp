#include "LogATE/Printers/detail/maxDigits.hpp"
#include <doctest/doctest.h>

using LogATE::Printers::detail::maxDigits;

namespace
{
TEST_SUITE("LogATE::Printers::detail::maxDigits")
{

TEST_CASE("corner cases")
{
  CHECK( 1 == maxDigits(0) );
  CHECK( 1 == maxDigits(1) );
  CHECK( 1 == maxDigits(9) );

  CHECK( 2 == maxDigits(10) );
  CHECK( 2 == maxDigits(11) );
  CHECK( 2 == maxDigits(99) );

  CHECK( 3 == maxDigits(100) );
  CHECK( 3 == maxDigits(101) );
  CHECK( 3 == maxDigits(999) );

  CHECK( 4 == maxDigits(1000) );
  CHECK( 4 == maxDigits(1001) );
  CHECK( 4 == maxDigits(9999) );

  CHECK( 5 == maxDigits(10000) );
  CHECK( 5 == maxDigits(10001) );
  CHECK( 5 == maxDigits(99999) );
}

}
}
