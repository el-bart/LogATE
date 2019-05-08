#include <doctest/doctest.h>
#include "CursATE/Screen/detail/formatAsPercentage.hpp"

using CursATE::Screen::detail::formatAsPercentage;
using namespace std::literals::string_literals;

namespace
{
TEST_SUITE("CursATE::Screen::detail::formatAsPercentage")
{


TEST_CASE("edge cases")
{
  CHECK( formatAsPercentage(0.0) == "  0.00%"s );
  CHECK( formatAsPercentage(1.0) == "100.00%"s );
}


TEST_CASE("small numbers")
{
  CHECK( formatAsPercentage(0.05) == "  5.00%"s );
}


TEST_CASE("decimal places")
{
  CHECK( formatAsPercentage(0.0505)  == "  5.05%"s );
  CHECK( formatAsPercentage(0.0525)  == "  5.25%"s );
  CHECK( formatAsPercentage(0.0525)  == "  5.25%"s );
  CHECK( formatAsPercentage(0.4525)  == " 45.25%"s );
}


TEST_CASE("extra decimal places are dropped")
{
  CHECK( formatAsPercentage(0.9999)    == " 99.99%"s );
  CHECK( formatAsPercentage(0.99999)   == " 99.99%"s );
  CHECK( formatAsPercentage(0.9999999) == " 99.99%"s );
}


TEST_CASE("bigger numbers")
{
  CHECK( formatAsPercentage(0.45) == " 45.00%"s );
  CHECK( formatAsPercentage(0.99) == " 99.00%"s );
}


TEST_CASE("out of 0-100% values")
{
  CHECK_THROWS_AS( formatAsPercentage(-0.5), CursATE::Screen::detail::NegativeValuesNotSupported );
  CHECK( formatAsPercentage(1.5)  ==  "150.00%"s );
  CHECK( formatAsPercentage(20.0) == "2000.00%"s );
}

}
}
