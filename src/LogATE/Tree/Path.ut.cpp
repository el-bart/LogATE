#include <doctest/doctest.h>
#include "LogATE/Tree/Path.hpp"

using LogATE::Tree::Path;

namespace
{

TEST_SUITE("Tree::Path")
{

TEST_CASE("isRoot() works")
{
  CHECK( Path{{}}.root() == true );
  CHECK( Path{{"."}}.root() == true );
  CHECK( Path{{".", "foo", "bar"}}.root() == true );
  CHECK( Path{{"oops", ".", "foo", "bar"}}.root() == false );
  CHECK( Path{{"foo", "bar"}}.root() == false );
}

}

}
