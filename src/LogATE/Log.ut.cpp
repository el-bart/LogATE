#include <doctest/doctest.h>
#include "LogATE/Log.hpp"

using LogATE::Log;
using LogATE::AnnotatedLog;
using SN = LogATE::SequenceNumber;

namespace
{
TEST_SUITE("Log")
{

TEST_CASE("c-tors with auto-sequence number")
{
  const auto str = std::string{ R"({"answer":42})" };
  SUBCASE("C-string")
  {
    const auto log = Log{ str.c_str() };
    CHECK( log.str() == str );
    CHECK( log.json() == nlohmann::json::parse(str) );
  }
  SUBCASE("std::string")
  {
    const auto log = Log{ str };
    CHECK( log.str() == str );
    CHECK( log.json() == nlohmann::json::parse(str) );
  }
  SUBCASE("json")
  {
    const auto log = Log{ nlohmann::json::parse(str) };
    CHECK( log.str() == str );
    CHECK( log.json() == nlohmann::json::parse(str) );
  }
}


TEST_CASE("c-tors with explicit sequence number")
{
  const auto sn = SN{42};
  const auto str = std::string{ R"({"answer":42})" };
  SUBCASE("C-string")
  {
    const auto log = Log{ sn, str.c_str() };
    CHECK( log.str() == str );
    CHECK( log.json() == nlohmann::json::parse(str) );
    CHECK( log.sequenceNumber() == sn );
  }
  SUBCASE("std::string")
  {
    const auto str = std::string{ R"({"answer":42})" };
    const auto log = Log{ sn, str };
    CHECK( log.str() == str );
    CHECK( log.json() == nlohmann::json::parse(str) );
    CHECK( log.sequenceNumber() == sn );
  }
  SUBCASE("json")
  {
    const auto str = std::string{ R"({"answer":42})" };
    const auto log = Log{ sn, nlohmann::json::parse(str) };
    CHECK( log.str() == str );
    CHECK( log.json() == nlohmann::json::parse(str) );
    CHECK( log.sequenceNumber() == sn );
  }
}


TEST_CASE("annotated log")
{
  const auto log = Log{ SN{42}, R"({"answer":42})" };
  const auto al = AnnotatedLog{log};
  CHECK( al.log_.str() == log.str() );
  CHECK( al.log_.sequenceNumber() == log.sequenceNumber() );
  CHECK( al.log_.json() == log.json() );
  CHECK( al.json_ == log.json() );
}

}
}
