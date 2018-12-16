#include <doctest/doctest.h>
#include <LogATE/placeholder.hpp>

namespace
{

TEST_CASE("valid is ok")
{
  CHECK( 42 == answer() );
}

}
