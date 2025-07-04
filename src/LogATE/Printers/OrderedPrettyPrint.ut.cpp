#include <doctest/doctest.h>
#include "LogATE/Printers/OrderedPrettyPrint.hpp"
#include "LogATE/TestHelpers.ut.hpp"

using LogATE::Printers::OrderedPrettyPrint;
using LogATE::makeLog;

namespace
{
TEST_SUITE("LogATE::Printers::OrderedPrettyPrint")
{

struct Fixture
{
  Fixture()
  {
    putSequenceNumberInExpectedRange();
  }

  void putSequenceNumberInExpectedRange()
  {
    while( LogATE::SequenceNumber::lastIssued().value_ < 10'000 )
      LogATE::SequenceNumber::next();
    REQUIRE( LogATE::SequenceNumber::lastIssued().value_ < 99'999 );
  }
};


TEST_CASE_FIXTURE(Fixture, "directly converting to pretty print")
{
  const OrderedPrettyPrint opp_;
  SUBCASE("basic types")
  {
    CHECK( "00013 some-key mystring=xxx" == opp_( makeLog(13, R"({"mystring": "xxx"})") ) );
    CHECK( "00013 some-key myint=42" == opp_( makeLog(13, R"({"myint":42})") ) );
    CHECK( "00013 some-key mybigint=1234567890" == opp_( makeLog(13, R"({"mybigint":1234567890})") ) );
    CHECK( "00013 some-key myfloat=4.5" == opp_( makeLog(13, R"({"myfloat":4.5})") ) );
    CHECK( "00013 some-key mysmallfloat=0.0005" == opp_( makeLog(13, R"({"mysmallfloat":0.0005})") ) );
    CHECK( "00013 some-key mybigfloat=123456789.5" == opp_( makeLog(13, R"({"mybigfloat":123456789.5})") ) );
    CHECK( "00013 some-key mybool=true" == opp_( makeLog(13, R"({"mybool":true})") ) );
  }
  SUBCASE("basic structures")
  {
    CHECK( "00013 some-key " == opp_( makeLog(13, R"({})") ) );
    CHECK( "00013 some-key foo=bar" == opp_( makeLog(13, R"({"foo":"bar"})") ) );
    const auto dualFields = opp_( makeLog(13, R"({"foo":"bar", "answer":42})") );
    if( "00013 some-key foo=bar answer=42" != dualFields && "00013 some-key answer=42 foo=bar" != dualFields )
    {
      CHECK( "00013 some-key foo=bar answer=42" == dualFields );
      CHECK( "00013 some-key answer=42 foo=bar" == dualFields );
    }
  }
  SUBCASE("nested structures")
  {
    CHECK( "00013 some-key answer={ expected={ } }" == opp_( makeLog(13, R"({"answer":{ "expected":{} }})") ) );
    CHECK( "00013 some-key answer={ expected=42 }" == opp_( makeLog(13, R"({"answer":{ "expected":42 }})") ) );
    CHECK( "00013 some-key narf={ fran={ xxx=42 } }" == opp_( makeLog(13, R"({"narf": { "fran": { "xxx":42 } } })") ) );
  }
}


TEST_CASE_FIXTURE(Fixture, "converting with priorities")
{
  const OrderedPrettyPrint::Config cfg{
    .priorityTags_ = OrderedPrettyPrint::PriorityTags{{"foo", "bar"}}
  };
  const OrderedPrettyPrint opp_{cfg};
  SUBCASE("root elements")
  {
    CHECK( "00013 some-key narf=xxx" == opp_( makeLog(13, R"({"narf": "xxx"})") ) );
    CHECK( "00013 some-key foo=xxx narf=yyy" == opp_( makeLog(13, R"({"narf": "yyy", "foo":"xxx"})") ) );
    CHECK( "00013 some-key bar=xxx narf=yyy" == opp_( makeLog(13, R"({"narf": "yyy", "bar":"xxx"})") ) );
    CHECK( "00013 some-key foo=xxx bar=zzz narf=yyy" == opp_( makeLog(13, R"({"narf": "yyy", "bar":"zzz", "foo":"xxx"})") ) );
  }
  SUBCASE("nested elements tags")
  {
    CHECK( "00013 some-key root={ narf=xxx }" == opp_( makeLog(13, R"({"root": {"narf": "xxx"} })") ) );
    CHECK( "00013 some-key root={ foo=xxx narf=yyy }" == opp_( makeLog(13, R"({"root": {"narf": "yyy", "foo":"xxx"} })") ) );
    CHECK( "00013 some-key root={ bar=xxx narf=yyy }" == opp_( makeLog(13, R"({"root": {"narf": "yyy", "bar":"xxx"} })") ) );
    CHECK( "00013 some-key root={ foo=xxx bar=zzz narf=yyy }" == opp_( makeLog(13, R"({"root": {"narf": "yyy", "bar":"zzz", "foo":"xxx"} })") ) );
  }
  SUBCASE("long input JSON")
  {
    CHECK( "00013 some-key foo=#9 " == opp_( makeLog(13, R"({
                                                 "00": "11",
                                                 "11": "22",
                                                 "22": "33",
                                                 "33": "44",
                                                 "44": "55",
                                                 "55": "66",
                                                 "66": "77",
                                                 "77": "88",
                                                 "88": "99",
                                                 "99": "00",
                                                 "narf": "xxx",
                                                 "abc": "#0",
                                                 "def": "#1",
                                                 "ghi": "#2",
                                                 "jkl": "#3",
                                                 "mno": "#4",
                                                 "pqr": "#5",
                                                 "stw": "#6",
                                                 "vxyz": "#7",
                                                 "01234": "#8",
                                                 "foo": "#9"
                                               })") ).substr(0,22) );
  }
}


TEST_CASE_FIXTURE(Fixture, "silent tags")
{
  const OrderedPrettyPrint::Config cfg{
    .silentTags_ = OrderedPrettyPrint::SilentTags{{"foo", "bar"}}
  };
  const OrderedPrettyPrint opp_{cfg};
  SUBCASE("root elements")
  {
    CHECK( "00013 some-key narf=xxx" == opp_( makeLog(13, R"({"narf": "xxx"})") ) );
    CHECK( "00013 some-key xxx" == opp_( makeLog(13, R"({"bar": "xxx"})") ) );
    CHECK( "00013 some-key xxx" == opp_( makeLog(13, R"({"foo": "xxx"})") ) );
  }
  SUBCASE("nested elements tags")
  {
    CHECK( "00013 some-key root={ narf=xxx }" == opp_( makeLog(13, R"({"root": {"narf": "xxx"} })") ) );
    CHECK( "00013 some-key root={ xxx }" == opp_( makeLog(13, R"({"root": {"bar": "xxx"} })") ) );
    CHECK( "00013 some-key root={ xxx }" == opp_( makeLog(13, R"({"root": {"foo": "xxx"} })") ) );
    CHECK( "00013 some-key { narf=xxx }" == opp_( makeLog(13, R"({"bar": {"narf": "xxx"} })") ) );
  }
}


TEST_CASE_FIXTURE(Fixture, "padded fields")
{
  const OrderedPrettyPrint::Config cfg{
    .paddedFields_ = OrderedPrettyPrint::PaddedFields{{ {"narf", 5} }}
  };
  const OrderedPrettyPrint opp_{cfg};
  SUBCASE("root elements")
  {
    CHECK( "00013 some-key narf=  xxx" == opp_( makeLog(13, R"({"narf": "xxx"})") ) );
    CHECK( "00013 some-key narf=12345" == opp_( makeLog(13, R"({"narf": "12345"})") ) );
  }
  SUBCASE("nested elements tags")
  {
    CHECK( "00013 some-key root={ narf=  xxx }" == opp_( makeLog(13, R"({"root": {"narf": "xxx"} })") ) );
    CHECK( "00013 some-key root={ narf=   42 }" == opp_( makeLog(13, R"({"root": {"narf": 42} })") ) );
    CHECK( "00013 some-key root={ narf= true }" == opp_( makeLog(13, R"({"root": {"narf": true} })") ) );
    CHECK( "00013 some-key root={ narf=  4.2 }" == opp_( makeLog(13, R"({"root": {"narf": 4.2} })") ) );
  }
  SUBCASE("non-leaf elements are not affected by padding")
  {
    CHECK( "00013 some-key narf={ narf=  xxx }" == opp_( makeLog(13, R"({"narf": {"narf": "xxx"} })") ) );
  }
  SUBCASE("padding does not last for other fields")
  {
    CHECK( "00013 some-key a={ narf=  def } b=def c={ narf=  abc }" == opp_( makeLog(13, R"({ "a": {"narf": "def"}, "b": "def", "c": {"narf": "abc"} })") ) );
  }
}


TEST_CASE_FIXTURE(Fixture, "silent tags with priorities and padded fields")
{
  const OrderedPrettyPrint::Config cfg{
    .silentTags_ = OrderedPrettyPrint::SilentTags{{"foo", "bar"}},
    .priorityTags_ = OrderedPrettyPrint::PriorityTags{{"foo", "narf"}},
    .paddedFields_ = OrderedPrettyPrint::PaddedFields{{ {"narf", 5} }}
  };
  const OrderedPrettyPrint opp_{cfg};
  CHECK( "00013 some-key xxx narf=  yyy zzz" == opp_( makeLog(13, R"({"narf": "yyy", "bar":"zzz", "foo":"xxx"})") ) );
}


TEST_CASE_FIXTURE(Fixture, "non-printable characters are converted")
{
  const OrderedPrettyPrint opp_;
  nlohmann::json json;
  json["xx\1\t"] = "x\r\x3xx";
  const auto log = LogATE::Log{ LogATE::Log::Key{"foo", LogATE::SequenceNumber{13}}, json };
  CHECK( "00013 foo xx\\x01\\t=x\\r\\x03xx" == opp_(log) );
}


TEST_CASE_FIXTURE(Fixture, "converting arrays")
{
  const OrderedPrettyPrint opp_;
  SUBCASE("root element")
  {
    CHECK( "00013 some-key [ ]" == opp_( makeLog(13, R"([])") ) );
    CHECK( "00013 some-key [ 1 ]" == opp_( makeLog(13, R"([1])") ) );
    CHECK( "00013 some-key [ 1 2 3 ]" == opp_( makeLog(13, R"([1,2,3])") ) );
  }
  SUBCASE("member element")
  {
    CHECK( "00013 some-key foo=[ ]" == opp_( makeLog(13, R"({ "foo": [] })") ) );
    CHECK( "00013 some-key foo=[ 1 ]" == opp_( makeLog(13, R"({ "foo": [1] })") ) );
    CHECK( "00013 some-key foo=[ 1 2 3 ]" == opp_( makeLog(13, R"({ "foo": [1, 2, 3] })") ) );
  }
  SUBCASE("nested element")
  {
    CHECK( "00013 some-key foo=[ { bar=[ 1 2 ] } { narf=[ 42 13 ] } ]" == opp_( makeLog(13, R"({ "foo": [ { "bar": [ 1,2] }, { "narf": [42,13] } ] })") ) );
    CHECK( "00013 some-key foo=[ 1 ]" == opp_( makeLog(13, R"({ "foo": [1] })") ) );
    CHECK( "00013 some-key foo=[ 1 2 3 ]" == opp_( makeLog(13, R"({ "foo": [1, 2, 3] })") ) );
  }
}

}
}
