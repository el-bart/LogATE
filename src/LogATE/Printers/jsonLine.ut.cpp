#include <doctest/doctest.h>
#include "LogATE/Printers/jsonLine.hpp"

using LogATE::Printers::jsonLine;

namespace
{
TEST_SUITE("LogATE::Printers::jsonLine")
{

TEST_CASE("converting to json line")
{
  CHECK( jsonLine( LogATE::json2log(R"("xxx")") ) == R"("xxx")" );
  CHECK( jsonLine( LogATE::json2log(R"({ "foo": "bar" })") ) == R"({"foo":"bar"})" );
  CHECK( jsonLine( LogATE::json2log(R"({ "foo": "bar", "n": 42 })") ) == R"({"foo":"bar","n":42})" );
}

}
}
