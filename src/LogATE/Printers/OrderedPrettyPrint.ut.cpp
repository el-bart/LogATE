#include <doctest/doctest.h>
#include "LogATE/Printers/OrderedPrettyPrint.hpp"

using LogATE::Printers::OrderedPrettyPrint;
using LogATE::json2log;

namespace
{
TEST_SUITE("LogATE::Printers::OrderedPrettyPrint")
{


TEST_CASE("directly converting to pretty print")
{
  const OrderedPrettyPrint opp_;
  SUBCASE("basic types")
  {
    CHECK( "mystring=xxx" == opp_( json2log(R"({"mystring": "xxx"})") ) );
    CHECK( "myint=42" == opp_( json2log(R"({"myint":42})") ) );
    CHECK( "mybigint=1234567890" == opp_( json2log(R"({"mybigint":1234567890})") ) );
    CHECK( "myfloat=4.5" == opp_( json2log(R"({"myfloat":4.5})") ) );
    CHECK( "mysmallfloat=0.0005" == opp_( json2log(R"({"mysmallfloat":0.0005})") ) );
    CHECK( "mybigfloat=123456789.5" == opp_( json2log(R"({"mybigfloat":123456789.5})") ) );
    CHECK( "mybool=true" == opp_( json2log(R"({"mybool":true})") ) );
  }
  SUBCASE("basic structures")
  {
    CHECK( "" == opp_( json2log(R"({})") ) );
    CHECK( "foo=bar" == opp_( json2log(R"({"foo":"bar"})") ) );
    const auto dualFields = opp_( json2log(R"({"foo":"bar", "answer":42})") );
    if( "foo=bar answer=42" != dualFields && "answer=42 foo=bar" != dualFields )
    {
      CHECK( "foo=bar answer=42" == dualFields );
      CHECK( "answer=42 foo=bar" == dualFields );
    }
  }
  SUBCASE("nested structures")
  {
    CHECK( "answer={ expected=42 }" == opp_( json2log(R"({"answer":{ "expected":42 }})") ) );
    CHECK( "narf={ fran={ xxx=42 } }" == opp_( json2log(R"({"narf": { "fran": { "xxx":42 } } })") ) );
  }
}


TEST_CASE("converting with priorities")
{
  const OrderedPrettyPrint opp_{ OrderedPrettyPrint::PriorityTags{{"foo", "bar"}} };
  SUBCASE("root elements")
  {
    CHECK( "narf=xxx" == opp_( json2log(R"({"narf": "xxx"})") ) );
    CHECK( "foo=xxx narf=yyy" == opp_( json2log(R"({"narf": "yyy", "foo":"xxx"})") ) );
    CHECK( "bar=xxx narf=yyy" == opp_( json2log(R"({"narf": "yyy", "bar":"xxx"})") ) );
    CHECK( "foo=xxx bar=zzz narf=yyy" == opp_( json2log(R"({"narf": "yyy", "bar":"zzz", "foo":"xxx"})") ) );
  }
  SUBCASE("nested elements tags")
  {
    CHECK( "root={ narf=xxx }" == opp_( json2log(R"({"root": {"narf": "xxx"} })") ) );
    CHECK( "root={ foo=xxx narf=yyy }" == opp_( json2log(R"({"root": {"narf": "yyy", "foo":"xxx"} })") ) );
    CHECK( "root={ bar=xxx narf=yyy }" == opp_( json2log(R"({"root": {"narf": "yyy", "bar":"xxx"} })") ) );
    CHECK( "root={ foo=xxx bar=zzz narf=yyy }" == opp_( json2log(R"({"root": {"narf": "yyy", "bar":"zzz", "foo":"xxx"} })") ) );
  }
}


TEST_CASE("silent tags")
{
  const OrderedPrettyPrint opp_{ OrderedPrettyPrint::SilentTags{{"foo", "bar"}} };
  SUBCASE("root elements")
  {
    CHECK( "narf=xxx" == opp_( json2log(R"({"narf": "xxx"})") ) );
    CHECK( "xxx" == opp_( json2log(R"({"bar": "xxx"})") ) );
    CHECK( "xxx" == opp_( json2log(R"({"foo": "xxx"})") ) );
  }
  SUBCASE("nested elements tags")
  {
    CHECK( "root={ narf=xxx }" == opp_( json2log(R"({"root": {"narf": "xxx"} })") ) );
    CHECK( "root={ xxx }" == opp_( json2log(R"({"root": {"bar": "xxx"} })") ) );
    CHECK( "root={ xxx }" == opp_( json2log(R"({"root": {"foo": "xxx"} })") ) );
    CHECK( "{ narf=xxx }" == opp_( json2log(R"({"bar": {"narf": "xxx"} })") ) );
  }
}


TEST_CASE("silent tags with priorities")
{
  const OrderedPrettyPrint opp_{ OrderedPrettyPrint::SilentTags{{"foo", "bar"}}, OrderedPrettyPrint::PriorityTags{{"foo", "narf"}} };
  CHECK( "xxx narf=yyy zzz" == opp_( json2log(R"({"narf": "yyy", "bar":"zzz", "foo":"xxx"})") ) );
}

}
}
