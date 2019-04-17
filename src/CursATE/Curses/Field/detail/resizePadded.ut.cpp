#include <doctest/doctest.h>
#include "CursATE/Curses/Field/detail/resizePadded.hpp"
#include "CursATE/Curses/TestPrints.ut.hpp"

using CursATE::Curses::Field::detail::resizePadded;
using CursATE::Curses::Field::detail::resizePaddedVisibleSize;
using CursATE::Curses::Field::detail::SelectionOutOfRange;
using std::string;

namespace
{
TEST_SUITE("CursATE::Curses::Field::detail:resizePadded")
{

TEST_CASE("strings shorter than the limit")
{
  CHECK( resizePadded("",     4, 0) == "    " );
  CHECK( resizePadded("a",    4, 0) == "a   " );
  CHECK( resizePadded("ab",   4, 0) == "ab  " );
  CHECK( resizePadded("abc",  4, 0) == "abc " );
  CHECK( resizePadded("abcd", 4, 0) == "abcd" );
}


TEST_CASE("strings longer than the limit")
{
  CHECK( resizePadded("abcdefg", 4, 0) == "abcd" );
}


TEST_CASE("selecting out of range throws")
{
  CHECK_THROWS_AS( resizePadded("abc", 2, 4), SelectionOutOfRange );
}


TEST_CASE("selection tries to keep around center of a text")
{
  SUBCASE("odd size")
  {
    CHECK( resizePadded("0123456789", 3,  0) == "012" );
    CHECK( resizePadded("0123456789", 3,  1) == "012" );
    CHECK( resizePadded("0123456789", 3,  2) == "123" );
    CHECK( resizePadded("0123456789", 3,  3) == "234" );
    CHECK( resizePadded("0123456789", 3,  7) == "678" );
    CHECK( resizePadded("0123456789", 3,  8) == "789" );
    CHECK( resizePadded("0123456789", 3,  9) == "789" );
    CHECK( resizePadded("0123456789", 3, 10) == "789" );
  }

  SUBCASE("even size")
  {
    CHECK( resizePadded("0123456789", 4,  0) == "0123" );
    CHECK( resizePadded("0123456789", 4,  1) == "0123" );
    CHECK( resizePadded("0123456789", 4,  2) == "0123" );
    CHECK( resizePadded("0123456789", 4,  3) == "1234" );
    CHECK( resizePadded("0123456789", 4,  6) == "4567" );
    CHECK( resizePadded("0123456789", 4,  7) == "5678" );
    CHECK( resizePadded("0123456789", 4,  8) == "6789" );
    CHECK( resizePadded("0123456789", 4,  9) == "6789" );
    CHECK( resizePadded("0123456789", 4, 10) == "6789" );
  }
}

TEST_CASE("edge cases on a very short strings")
{
  SUBCASE("window size == 0")
  {
    CHECK( resizePadded("0123456789", 0,  0) == "" );
    CHECK( resizePadded("0123456789", 0,  5) == "" );
    CHECK( resizePadded("0123456789", 0,  9) == "" );
    CHECK( resizePadded("0123456789", 0, 10) == "" );
  }
  SUBCASE("window size == 1")
  {
    CHECK( resizePadded("0123456789", 1,  0) == "0" );
    CHECK( resizePadded("0123456789", 1,  1) == "1" );
    CHECK( resizePadded("0123456789", 1,  5) == "5" );
    CHECK( resizePadded("0123456789", 1,  8) == "8" );
    CHECK( resizePadded("0123456789", 1,  9) == "9" );
    CHECK( resizePadded("0123456789", 1, 10) == "9" );
  }
  SUBCASE("window size == 2")
  {
    CHECK( resizePadded("0123456789", 2,  0) == "01" );
    CHECK( resizePadded("0123456789", 2,  1) == "01" );
    CHECK( resizePadded("0123456789", 2,  2) == "12" );
    CHECK( resizePadded("0123456789", 2,  5) == "45" );
    CHECK( resizePadded("0123456789", 2,  7) == "67" );
    CHECK( resizePadded("0123456789", 2,  8) == "78" );
    CHECK( resizePadded("0123456789", 2,  9) == "89" );
    CHECK( resizePadded("0123456789", 2, 10) == "89" );
  }
}

TEST_CASE("check screen position offsets")
{
  SUBCASE("window size == 0")
  {
    CHECK( resizePaddedVisibleSize("0123456789", 0,  0).selectionOffset_ == 0 );
    CHECK( resizePaddedVisibleSize("0123456789", 0,  5).selectionOffset_ == 0 );
    CHECK( resizePaddedVisibleSize("0123456789", 0,  9).selectionOffset_ == 0 );
    CHECK( resizePaddedVisibleSize("0123456789", 0, 10).selectionOffset_ == 0 );
  }
  SUBCASE("window size == 1")
  {
    CHECK( resizePaddedVisibleSize("0123456789", 1,  0).selectionOffset_ == 0 );
    CHECK( resizePaddedVisibleSize("0123456789", 1,  1).selectionOffset_ == 0 );
    CHECK( resizePaddedVisibleSize("0123456789", 1,  5).selectionOffset_ == 0 );
    CHECK( resizePaddedVisibleSize("0123456789", 1,  8).selectionOffset_ == 0 );
    CHECK( resizePaddedVisibleSize("0123456789", 1,  9).selectionOffset_ == 0 );
    CHECK( resizePaddedVisibleSize("0123456789", 1, 10).selectionOffset_ == 1 );
  }
  SUBCASE("window size == 2")
  {
    CHECK( resizePaddedVisibleSize("0123456789", 2,  0).selectionOffset_ == 0 );
    CHECK( resizePaddedVisibleSize("0123456789", 2,  1).selectionOffset_ == 1 );
    CHECK( resizePaddedVisibleSize("0123456789", 2,  2).selectionOffset_ == 1 );
    CHECK( resizePaddedVisibleSize("0123456789", 2,  5).selectionOffset_ == 1 );
    CHECK( resizePaddedVisibleSize("0123456789", 2,  7).selectionOffset_ == 1 );
    CHECK( resizePaddedVisibleSize("0123456789", 2,  8).selectionOffset_ == 1 );
    CHECK( resizePaddedVisibleSize("0123456789", 2,  9).selectionOffset_ == 1 );
    CHECK( resizePaddedVisibleSize("0123456789", 2, 10).selectionOffset_ == 2 );
  }
  SUBCASE("string below screen size")
  {
    CHECK( resizePaddedVisibleSize("01234", 5, 0).selectionOffset_ == 0 );
    CHECK( resizePaddedVisibleSize("01234", 5, 1).selectionOffset_ == 1 );
    CHECK( resizePaddedVisibleSize("01234", 5, 2).selectionOffset_ == 2 );
    CHECK( resizePaddedVisibleSize("01234", 5, 3).selectionOffset_ == 3 );
    CHECK( resizePaddedVisibleSize("01234", 5, 4).selectionOffset_ == 4 );
  }
}

}
}
