#include <doctest/doctest.h>
#include "LogATE/Utils/forEachMatch.hpp"
#include <optional>

using LogATE::Utils::forEachMatch;
using LogATE::Tree::Path;

namespace
{
TEST_SUITE("Tree::Utils::forEachMatch")
{

struct Recorder
{
  bool operator()(nlohmann::json const& value)
  {
    ++callCount_;
    values_.push_back(&value);
    if(stopAfter_ && *stopAfter_ <= callCount_ )
      return false;
    return true;
  }

  std::vector<nlohmann::json const*> values_;
  std::optional<unsigned> stopAfter_{};
  unsigned callCount_{0};
};


struct Fixture
{
  Fixture()
  {
    assert( big_.is_object() && "nlohmann::json{ n::json{...} } created 1-element array!" );
  }

  Recorder rec_;
  const nlohmann::json big_ =
                            R"({
                                "one": {
                                  "PING": {
                                    "PONG": {
                                      "narf": 42
                                    }
                                  }
                                },
                                "two": {
                                  "PING": {
                                    "PONG": {
                                      "narf": {
                                        "fran": "aaa"
                                      }
                                    }
                                  }
                                },
                                "three": {
                                  "foo": {
                                    "bar": "xxx"
                                  }
                                },
                                "four": {
                                  "foo": {
                                    "bar": "yyy"
                                  }
                                },
                                "five": {
                                  "foo": [
                                    { "one": 1 },
                                    { "two": 2 }
                                  ]
                                },
                                "six": [
                                   13,
                                   42,
                                   69,
                                   51
                                 ]
                               })"_json;
  const nlohmann::json nested_ =
                            R"({
                                "meh": {
                                  "foo": [
                                      { "bar": [ 1, 2, 3 ] },
                                      { "bar": [ 42, 13 ] }
                                    ]
                                  }
                                })"_json;
};


TEST_CASE_FIXTURE(Fixture, "null json")
{
  const nlohmann::json null_{};
  CHECK( forEachMatch(null_, Path::parse(".one"), rec_) == true );
  REQUIRE( rec_.values_.size() == 0u );
}


TEST_CASE_FIXTURE(Fixture, "empty path")
{
  CHECK( forEachMatch(big_, Path{}, rec_) == true );
  REQUIRE( rec_.values_.size() == 0u );
}


TEST_CASE_FIXTURE(Fixture, "absolute path")
{
  SUBCASE("root")
  {
    CHECK( forEachMatch(big_, Path::parse("."), rec_) == true );
    REQUIRE( rec_.values_.size() == 1u );
    CHECK( rec_.values_[0] == &big_ );
  }
  SUBCASE("node")
  {
    CHECK( forEachMatch(big_, Path::parse(".four"), rec_) == true );
    REQUIRE( rec_.values_.size() == 1u );
    CHECK( rec_.values_[0] == &big_["four"] );
  }
  SUBCASE("non-existing node")
  {
    CHECK( forEachMatch(big_, Path::parse(".does.not.exist"), rec_) == true );
    REQUIRE( rec_.values_.size() == 0u );
  }
}


TEST_CASE_FIXTURE(Fixture, "absolute path starting with array")
{
  SUBCASE("direct index")
  {
    // TODO[array]
  }
  SUBCASE("wildcard")
  {
    // TODO[array]
  }
}


TEST_CASE_FIXTURE(Fixture, "relative path")
{
  SUBCASE("node")
  {
    CHECK( forEachMatch(big_, Path::parse("four"), rec_) == true );
    REQUIRE( rec_.values_.size() == 1u );
    CHECK( rec_.values_[0] == &big_["four"] );
  }
  SUBCASE("leaf")
  {
    CHECK( forEachMatch(big_, Path::parse("narf.fran"), rec_) == true );
    REQUIRE( rec_.values_.size() == 1u );
    CHECK( rec_.values_[0] == &big_["two"]["PING"]["PONG"]["narf"]["fran"] );
  }
  SUBCASE("non-existing node")
  {
    CHECK( forEachMatch(big_, Path::parse("four.does.not.exist"), rec_) == true );
    REQUIRE( rec_.values_.size() == 0u );
  }
  SUBCASE("multiple hits")
  {
    CHECK( forEachMatch(big_, Path::parse("PING"), rec_) == true );
    REQUIRE( rec_.values_.size() == 2u );
    CHECK( rec_.values_[0] == &big_["one"]["PING"] );
    CHECK( rec_.values_[1] == &big_["two"]["PING"] );
  }
}


TEST_CASE_FIXTURE(Fixture, "relative path with arrays")
{
  SUBCASE("node")
  {
    CHECK( forEachMatch(big_, Path::parse("foo[1]"), rec_) == true );
    REQUIRE( rec_.values_.size() == 1u );
    CHECK( rec_.values_[0] == &big_["five"]["foo"][1] );
  }
  SUBCASE("leaf")
  {
    CHECK( forEachMatch(big_, Path::parse("foo[0].one"), rec_) == true );
    REQUIRE( rec_.values_.size() == 1u );
    CHECK( rec_.values_[0] == &big_["five"]["foo"][0]["one"] );
  }
  SUBCASE("non-existing node")
  {
    CHECK( forEachMatch(big_, Path::parse("foo[42].not.exist"), rec_) == true );
    REQUIRE( rec_.values_.size() == 0u );
  }
}


TEST_CASE_FIXTURE(Fixture, "relative path with wildcard arrays")
{
  CHECK( forEachMatch(nested_, Path::parse("foo[].bar[]"), rec_) == true );
  REQUIRE( rec_.values_.size() == 5u );
  CHECK( rec_.values_[0] == &nested_["meh"]["foo"][0]["bar"][0] );
  CHECK( rec_.values_[1] == &nested_["meh"]["foo"][0]["bar"][1] );
  CHECK( rec_.values_[2] == &nested_["meh"]["foo"][0]["bar"][2] );
  CHECK( rec_.values_[3] == &nested_["meh"]["foo"][1]["bar"][0] );
  CHECK( rec_.values_[4] == &nested_["meh"]["foo"][1]["bar"][1] );
}


TEST_CASE_FIXTURE(Fixture, "check end of processing after returning false from functor")
{
  SUBCASE("absolute path")
  {
    rec_.stopAfter_ = 1;
    CHECK( forEachMatch(big_, Path::parse(".one"), rec_) == false );
    REQUIRE( rec_.values_.size() == 1u );
    CHECK( rec_.values_[0] == &big_["one"] );
  }
  SUBCASE("relative path")
  {
    rec_.stopAfter_ = 1;
    CHECK( forEachMatch(big_, Path::parse("one"), rec_) == false );
    REQUIRE( rec_.values_.size() == 1u );
    CHECK( rec_.values_[0] == &big_["one"] );
  }
  SUBCASE("multiple hits - stop after 1st hit")
  {
    rec_.stopAfter_ = 1;
    CHECK( forEachMatch(big_, Path::parse("PING"), rec_) == false );
    REQUIRE( rec_.values_.size() == 1u );
    CHECK( rec_.values_[0] == &big_["one"]["PING"] );
  }
  SUBCASE("multiple hits - stop after 2nd hit")
  {
    rec_.stopAfter_ = 2;
    CHECK( forEachMatch(big_, Path::parse("PING"), rec_) == false );
    REQUIRE( rec_.values_.size() == 2u );
    CHECK( rec_.values_[0] == &big_["one"]["PING"] );
    CHECK( rec_.values_[1] == &big_["two"]["PING"] );
  }
  // TODO[array]: based on array wildcards, too
  // TODO[array]: root element can be an array, too.
}

}
}
