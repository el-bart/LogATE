#include <doctest/doctest.h>
#include "LogATE/Printers/jsonLine.hpp"
#include "LogATE/TestHelpers.ut.hpp"

using LogATE::Printers::jsonLine;
using LogATE::makeLog;

namespace
{
TEST_SUITE("LogATE::Printers::jsonLine")
{

TEST_CASE("converting to json line")
{
  CHECK( jsonLine( makeLog(R"("xxx")") ) == R"("xxx")" );
  CHECK( jsonLine( makeLog(R"({ "foo": "bar" })") ) == R"({"foo":"bar"})" );
  CHECK( jsonLine( makeLog(R"({ "foo": "bar", "n": 42 })") ) == R"({"foo":"bar","n":42})" );
}

}
}
