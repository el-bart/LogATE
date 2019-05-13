#include <doctest/doctest.h>
#include "CursATE/Screen/detail/formatAsPercentage.hpp"

using CursATE::Screen::detail::formatAsPercentage;
using CursATE::Screen::detail::padLeftWithSpaces;
using CursATE::Screen::detail::nOFm;
using CursATE::Screen::detail::nOFmWithPercent;
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


TEST_CASE("left-padding")
{
  CHECK( padLeftWithSpaces("", 0) == "" );
  CHECK( padLeftWithSpaces("", 1) == " " );
  CHECK( padLeftWithSpaces("xxx", 0) == "xxx" );
  CHECK( padLeftWithSpaces("xxx", 1) == "xxx" );
  CHECK( padLeftWithSpaces("xxx", 2) == "xxx" );
  CHECK( padLeftWithSpaces("xxx", 3) == "xxx" );
  CHECK( padLeftWithSpaces("xxx", 4) == " xxx" );
  CHECK( padLeftWithSpaces("xxx", 5) == "  xxx" );
}


TEST_CASE("N of M formatting")
{
  CHECK( nOFm(0,0) == "0/0" );
  CHECK( nOFm(1,0) == "1/0" );
  CHECK( nOFm(99,0) == "99/0" );
  CHECK( nOFm(0,1) == "0/1" );
  CHECK( nOFm(0,10) == " 0/10" );
  CHECK( nOFm(2,10) == " 2/10" );
  CHECK( nOFm(0,123) == "  0/123" );
  CHECK( nOFm(123,123) == "123/123" );
  CHECK( nOFm(1234,123) == "1234/123" );
}


TEST_CASE("N of M with % formatting")
{
  CHECK( nOFmWithPercent(0,0) == "0/0 (100.00%)" );
  CHECK( nOFmWithPercent(1,0) == "1/0 (100.00%)" );
  CHECK( nOFmWithPercent(99,0) == "99/0 (100.00%)" );
  CHECK( nOFmWithPercent(0,1) == "0/1 (  0.00%)" );
  CHECK( nOFmWithPercent(0,10) == " 0/10 (  0.00%)" );
  CHECK( nOFmWithPercent(2,10) == " 2/10 ( 20.00%)" );
  CHECK( nOFmWithPercent(0,123) == "  0/123 (  0.00%)" );
  CHECK( nOFmWithPercent(123,123) == "123/123 (100.00%)" );
  CHECK( nOFmWithPercent(2460,123) == "2460/123 (2000.00%)" );
}

}
}
