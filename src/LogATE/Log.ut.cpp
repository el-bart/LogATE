#include <doctest/doctest.h>
#include "LogATE/Log.hpp"
#include "LogATE/TestHelpers.ut.hpp"

using LogATE::Log;
using LogATE::AnnotatedLog;
using LogATE::makeKey;
using SN = LogATE::SequenceNumber;
using Key = LogATE::Log::Key;

namespace
{
TEST_SUITE("Log")
{

TEST_CASE("c-tors with auto-sequence number")
{
  const auto str = std::string{ R"({"answer":42})" };
  SUBCASE("C-string")
  {
    const auto log = Log{ makeKey(), str.c_str() };
    CHECK( log.str() == str );
    CHECK( log.json() == nlohmann::json::parse(str) );
  }
  SUBCASE("std::string")
  {
    const auto log = Log{ makeKey(), str };
    CHECK( log.str() == str );
    CHECK( log.json() == nlohmann::json::parse(str) );
  }
  SUBCASE("json")
  {
    const auto log = Log{ makeKey(), nlohmann::json::parse(str) };
    CHECK( log.str() == str );
    CHECK( log.json() == nlohmann::json::parse(str) );
  }
}


TEST_CASE("c-tors with explicit sequence number")
{
  const auto key = makeKey(42, "kszy");
  const auto str = std::string{ R"({"answer":42})" };
  SUBCASE("C-string")
  {
    const auto log = Log{ key, str.c_str() };
    CHECK( log.str() == str );
    CHECK( log.json() == nlohmann::json::parse(str) );
    CHECK( log.sequenceNumber() == key.sequenceNumber() );
    CHECK( log.key() == key );
  }
  SUBCASE("std::string")
  {
    const auto str = std::string{ R"({"answer":42})" };
    const auto log = Log{ key, str };
    CHECK( log.str() == str );
    CHECK( log.json() == nlohmann::json::parse(str) );
    CHECK( log.sequenceNumber() == key.sequenceNumber() );
    CHECK( log.key() == key );
  }
  SUBCASE("json")
  {
    const auto str = std::string{ R"({"answer":42})" };
    const auto log = Log{ key, nlohmann::json::parse(str) };
    CHECK( log.str() == str );
    CHECK( log.json() == nlohmann::json::parse(str) );
    CHECK( log.sequenceNumber() == key.sequenceNumber() );
    CHECK( log.key() == key );
  }
}


TEST_CASE("annotated log")
{
  const auto log = Log{ makeKey(42), R"({"answer":42})" };
  const auto al = AnnotatedLog{log};
  CHECK( al.log().str() == log.str() );
  CHECK( al.log().sequenceNumber() == log.sequenceNumber() );
  CHECK( al.log().json() == log.json() );
  CHECK( al.json() == log.json() );
}


TEST_CASE("annotated log directly from string")
{
  const auto str = R"({"answer":42})";
  const auto keyExtractor = LogATE::Tree::KeyExtractor{ LogATE::Tree::Path::parse(".answer"), LogATE::Tree::KeyExtractor::SourceFormat::Raw };
  SUBCASE("valid JSON")
  {
    const auto al = AnnotatedLog{str, keyExtractor};
    CHECK( al.log().str() == str );
    CHECK( al.log().json() == nlohmann::json::parse(str) );
  }
  SUBCASE("invalid JSON throws")
  {
    CHECK_THROWS( (AnnotatedLog{ std::string{"["} + str, keyExtractor }) );
  }
  SUBCASE("empty JSON throws")
  {
    CHECK_THROWS( (AnnotatedLog{ "", keyExtractor }) );
    CHECK_THROWS( (AnnotatedLog{ "{}", keyExtractor }) );
    CHECK_THROWS( (AnnotatedLog{ "[]", keyExtractor }) );
  }
  SUBCASE("sequence numbers are assigned uniquely and consecutively")
  {
    const auto al1 = AnnotatedLog{str, keyExtractor};
    const auto al2 = AnnotatedLog{str, keyExtractor};
    CHECK( al1.log().sequenceNumber() < al2.log().sequenceNumber() );
  }
}


TEST_CASE("Key")
{
  SUBCASE("compare identical")
  {
    const auto a = Key{"foo", SN{42}};
    const auto b = Key{"foo", SN{42}};
    CHECK      (a==b);
    CHECK_FALSE(a!=b);
    CHECK_FALSE(a< b);
    CHECK_FALSE(a> b);
    CHECK      (a<=b);
    CHECK      (a>=b);
  }
  SUBCASE("compare identical strings with different SNs")
  {
    const auto a = Key{"foo", SN{13}};
    const auto b = Key{"foo", SN{42}};
    CHECK_FALSE(a==b);
    CHECK      (a!=b);
    CHECK      (a< b);
    CHECK_FALSE(a> b);
    CHECK      (a<=b);
    CHECK_FALSE(a>=b);
  }
  SUBCASE("compare non-equal by equal-lenght strings (A)")
  {
    const auto a = Key{"bar", SN{42}};
    const auto b = Key{"foo", SN{13}};
    CHECK_FALSE(a==b);
    CHECK      (a!=b);
    CHECK      (a< b);
    CHECK_FALSE(a> b);
    CHECK      (a<=b);
    CHECK_FALSE(a>=b);
  }
  SUBCASE("compare non-equal by equal-lenght strings (B)")
  {
    const auto a = Key{"bar", SN{66}};
    const auto b = Key{"foo", SN{99}};
    CHECK_FALSE(a==b);
    CHECK      (a!=b);
    CHECK      (a< b);
    CHECK_FALSE(a> b);
    CHECK      (a<=b);
    CHECK_FALSE(a>=b);
  }
  SUBCASE("compare non-equal by non-equal-lenght strings (A)")
  {
    const auto a = Key{"bar", SN{42}};
    const auto b = Key{"f",   SN{13}};
    CHECK_FALSE(a==b);
    CHECK      (a!=b);
    CHECK      (a< b);
    CHECK_FALSE(a> b);
    CHECK      (a<=b);
    CHECK_FALSE(a>=b);
  }
  SUBCASE("compare non-equal by non-equal-lenght strings (B)")
  {
    const auto a = Key{"bar", SN{13}};
    const auto b = Key{"f",   SN{42}};
    CHECK_FALSE(a==b);
    CHECK      (a!=b);
    CHECK      (a< b);
    CHECK_FALSE(a> b);
    CHECK      (a<=b);
    CHECK_FALSE(a>=b);
  }
}

}
}
