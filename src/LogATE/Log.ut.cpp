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
  CHECK( al.log().str() == log.str() );
  CHECK( al.log().sequenceNumber() == log.sequenceNumber() );
  CHECK( al.log().json() == log.json() );
  CHECK( al.json() == log.json() );
}


TEST_CASE("annotated log directly from string")
{
  const auto str = R"({"answer":42})";
  SUBCASE("valid JSON")
  {
    const auto al = AnnotatedLog{str};
    CHECK( al.log().str() == str );
    CHECK( al.log().json() == nlohmann::json::parse(str) );
  }
  SUBCASE("invalid JSON throws")
  {
    CHECK_THROWS( AnnotatedLog{ std::string{"["} + str} );
  }
  SUBCASE("sequence numbers are assigned uniquely and consecutively")
  {
    const auto al1 = AnnotatedLog{str};
    const auto al2 = AnnotatedLog{str};
    CHECK( al1.log().sequenceNumber() < al2.log().sequenceNumber() );
  }
}

}
}
