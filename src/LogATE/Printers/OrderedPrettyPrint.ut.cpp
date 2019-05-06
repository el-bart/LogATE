#include <doctest/doctest.h>
#include "LogATE/Printers/OrderedPrettyPrint.hpp"

using LogATE::Printers::OrderedPrettyPrint;
using LogATE::json2log;

namespace
{
TEST_SUITE("LogATE::Printers::OrderedPrettyPrint")
{

auto mklog(const unsigned sn, std::string const& json)
{
  auto log = json2log(json);
  log.sn_.value_ = sn;
  return log;
}

TEST_CASE("directly converting to pretty print")
{
  const OrderedPrettyPrint opp_;
  SUBCASE("basic types")
  {
    CHECK( "13 mystring=xxx" == opp_( mklog(13, R"({"mystring": "xxx"})") ) );
    CHECK( "13 myint=42" == opp_( mklog(13, R"({"myint":42})") ) );
    CHECK( "13 mybigint=1234567890" == opp_( mklog(13, R"({"mybigint":1234567890})") ) );
    CHECK( "13 myfloat=4.5" == opp_( mklog(13, R"({"myfloat":4.5})") ) );
    CHECK( "13 mysmallfloat=0.0005" == opp_( mklog(13, R"({"mysmallfloat":0.0005})") ) );
    CHECK( "13 mybigfloat=123456789.5" == opp_( mklog(13, R"({"mybigfloat":123456789.5})") ) );
    CHECK( "13 mybool=true" == opp_( mklog(13, R"({"mybool":true})") ) );
  }
  SUBCASE("basic structures")
  {
    CHECK( "13 " == opp_( mklog(13, R"({})") ) );
    CHECK( "13 foo=bar" == opp_( mklog(13, R"({"foo":"bar"})") ) );
    const auto dualFields = opp_( mklog(13, R"({"foo":"bar", "answer":42})") );
    if( "13 foo=bar answer=42" != dualFields && "13 answer=42 foo=bar" != dualFields )
    {
      CHECK( "13 foo=bar answer=42" == dualFields );
      CHECK( "13 answer=42 foo=bar" == dualFields );
    }
  }
  SUBCASE("nested structures")
  {
    CHECK( "13 answer={ expected=42 }" == opp_( mklog(13, R"({"answer":{ "expected":42 }})") ) );
    CHECK( "13 narf={ fran={ xxx=42 } }" == opp_( mklog(13, R"({"narf": { "fran": { "xxx":42 } } })") ) );
  }
}


TEST_CASE("converting with priorities")
{
  const OrderedPrettyPrint opp_{ OrderedPrettyPrint::PriorityTags{{"foo", "bar"}} };
  SUBCASE("root elements")
  {
    CHECK( "13 narf=xxx" == opp_( mklog(13, R"({"narf": "xxx"})") ) );
    CHECK( "13 foo=xxx narf=yyy" == opp_( mklog(13, R"({"narf": "yyy", "foo":"xxx"})") ) );
    CHECK( "13 bar=xxx narf=yyy" == opp_( mklog(13, R"({"narf": "yyy", "bar":"xxx"})") ) );
    CHECK( "13 foo=xxx bar=zzz narf=yyy" == opp_( mklog(13, R"({"narf": "yyy", "bar":"zzz", "foo":"xxx"})") ) );
  }
  SUBCASE("nested elements tags")
  {
    CHECK( "13 root={ narf=xxx }" == opp_( mklog(13, R"({"root": {"narf": "xxx"} })") ) );
    CHECK( "13 root={ foo=xxx narf=yyy }" == opp_( mklog(13, R"({"root": {"narf": "yyy", "foo":"xxx"} })") ) );
    CHECK( "13 root={ bar=xxx narf=yyy }" == opp_( mklog(13, R"({"root": {"narf": "yyy", "bar":"xxx"} })") ) );
    CHECK( "13 root={ foo=xxx bar=zzz narf=yyy }" == opp_( mklog(13, R"({"root": {"narf": "yyy", "bar":"zzz", "foo":"xxx"} })") ) );
  }
}


TEST_CASE("silent tags")
{
  const OrderedPrettyPrint opp_{ OrderedPrettyPrint::SilentTags{{"foo", "bar"}} };
  SUBCASE("root elements")
  {
    CHECK( "13 narf=xxx" == opp_( mklog(13, R"({"narf": "xxx"})") ) );
    CHECK( "13 xxx" == opp_( mklog(13, R"({"bar": "xxx"})") ) );
    CHECK( "13 xxx" == opp_( mklog(13, R"({"foo": "xxx"})") ) );
  }
  SUBCASE("nested elements tags")
  {
    CHECK( "13 root={ narf=xxx }" == opp_( mklog(13, R"({"root": {"narf": "xxx"} })") ) );
    CHECK( "13 root={ xxx }" == opp_( mklog(13, R"({"root": {"bar": "xxx"} })") ) );
    CHECK( "13 root={ xxx }" == opp_( mklog(13, R"({"root": {"foo": "xxx"} })") ) );
    CHECK( "13 { narf=xxx }" == opp_( mklog(13, R"({"bar": {"narf": "xxx"} })") ) );
  }
}


TEST_CASE("silent tags with priorities")
{
  const OrderedPrettyPrint opp_{ OrderedPrettyPrint::SilentTags{{"foo", "bar"}}, OrderedPrettyPrint::PriorityTags{{"foo", "narf"}} };
  CHECK( "13 xxx narf=yyy zzz" == opp_( mklog(13, R"({"narf": "yyy", "bar":"zzz", "foo":"xxx"})") ) );
}

}
}
