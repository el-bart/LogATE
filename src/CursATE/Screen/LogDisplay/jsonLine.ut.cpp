#include <doctest/doctest.h>
#include "CursATE/Screen/LogDisplay/jsonLine.hpp"

using CursATE::Screen::LogDisplay::jsonLine;

namespace
{
TEST_SUITE("CursATE::Screen::LogDisplay::jsonLine")
{

TEST_CASE("converting to json line")
{
  CHECK( jsonLine( LogATE::json2log(R"("xxx")") ) == R"("xxx")" );
  CHECK( jsonLine( LogATE::json2log(R"({ "foo": "bar" })") ) == R"({"foo":"bar"})" );
  CHECK( jsonLine( LogATE::json2log(R"({ "foo": "bar", "n": 42 })") ) == R"({"foo":"bar","n":42})" );
}

}
}
