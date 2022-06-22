#include <doctest/doctest.h>
#include "LogATE/Utils/matchesLog.hpp"
#include "LogATE/TestHelpers.ut.hpp"

using LogATE::Log;
using LogATE::AnnotatedLog;
using LogATE::Tree::Path;
using LogATE::makeKey;
using LogATE::makeLog;
using LogATE::Utils::matchesKey;
using LogATE::Utils::matchesValue;
using LogATE::Utils::matchesAnyKey;
using LogATE::Utils::matchesAnyValue;
using LogATE::Utils::matchesAnyKeyValue;
using LogATE::Utils::allValues;
using LogATE::Utils::allNodeValues;
using LogATE::Utils::g_defaultRegexType;

namespace
{
TEST_SUITE("Tree::Utils::matchesLog")
{

struct Fixture
{
  auto testMatch(Path const& path, std::string const& re, Log const& log) const
  {
    const std::regex reg{re, g_defaultRegexType};
    REQUIRE(matchFunction);
    return (*matchFunction)( AnnotatedLog{log}, path, reg );
  }

  auto testMatch(Path const& path, std::string const& re) const
  {
    return testMatch(path, re, log_);
  }
  auto testMatchMulti(Path const& path, std::string const& re) const
  {
    return testMatch(path, re, logMulti_);
  }

  const Log log_{ makeKey(), R"({
                                "PING": {
                                  "PONG": {
                                    "narf": {
                                      "fran": "a_c"
                                    }
                                  }
                                },
                                "foo": {
                                  "bar": "a/c"
                                },
                                "array": [
                                  { "one": 1 },
                                  { "two": 2 }
                                ]
                              })" };
  const Log logMulti_{ makeKey(), R"({
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
                                }
                              })" };
  bool (*matchFunction)(AnnotatedLog const&, Path const&, std::regex const&) = nullptr;
};


TEST_CASE_FIXTURE(Fixture, "converting all basic types for value comparison")
{
  matchFunction = matchesValue;
  SUBCASE("string")
  {
    const auto log = makeLog(R"({ "val": "axxx" })");
    CHECK( testMatch(Path{{".", "val"}}, "ax*", log) );
  }
  SUBCASE("int")
  {
    const auto log = makeLog(R"({ "val": 42 })");
    CHECK( testMatch(Path{{".", "val"}}, "^42$", log) );
  }
  SUBCASE("float")
  {
    const auto log = makeLog(R"({ "val": 4.0 })");
    CHECK( testMatch(Path{{".", "val"}}, "^4.0*", log) );
  }
  SUBCASE("bool")
  {
    const auto log = makeLog(R"({ "val": true })");
    CHECK( testMatch(Path{{".", "val"}}, "true", log) );
  }
  SUBCASE("regex matches in the middle as well")
  {
    const auto log = makeLog(R"({ "val": "foo-xxx-bar" })");
    CHECK( testMatch(Path{{".", "val"}}, "xxx", log) );
  }
}


TEST_CASE_FIXTURE(Fixture, "value comparison of absolute path with regexs")
{
  matchFunction = matchesValue;
  CHECK( testMatch( Path{{".", "foo", "bar"}},         "a"    ) == true  );
  CHECK( testMatch( Path{{".", "foo", "bar"}},         "a.*c" ) == true  );
  CHECK( testMatch( Path{{".", "FOO", "BAR"}},         "a.*c" ) == false );
  CHECK( testMatch( Path{{".", "foo"}},                "b.*r" ) == false );
  CHECK( testMatch( Path{{".", "no", "such", "node"}}, "a.*c" ) == false );
  CHECK( testMatch( Path{{"."}},                       "a.*c" ) == false );
  CHECK( testMatch( Path{{".", "array[0]", "one"}},    "1"    ) == true  );
  CHECK( testMatch( Path{{".", "array[1]", "one"}},    "1"    ) == false );
}


TEST_CASE_FIXTURE(Fixture, "value comparison of absolute path")
{
  matchFunction = matchesValue;
  CHECK( testMatch( Path{{".", "foo", "bar"}},         "a/c" ) == true  );
  CHECK( testMatch( Path{{".", "foo", "bar"}},         "A/C" ) == false );
  CHECK( testMatch( Path{{".", "FOO", "BAR"}},         "a/c" ) == false );
  CHECK( testMatch( Path{{".", "foo"}},                "bar" ) == false );
  CHECK( testMatch( Path{{".", "no", "such", "node"}}, "a=c" ) == false );
  CHECK( testMatch( Path{{"."}},                       "a=c" ) == false );
}


TEST_CASE_FIXTURE(Fixture, "value comparison of relative path")
{
  matchFunction = matchesValue;
  CHECK( testMatch( Path{{"fran"}},               "a_c" ) == true  );
  CHECK( testMatch( Path{{"fran"}},               "A_C" ) == false );
  CHECK( testMatch( Path{{"narf", "fran"}},       "a_c" ) == true  );
  CHECK( testMatch( Path{{"narf", "fran"}},       "A_C" ) == false );
  CHECK( testMatch( Path{{"NARF", "FRAN"}},       "a_c" ) == false );
  CHECK( testMatch( Path{{"ping", "fran"}},       "a_c" ) == false );
  CHECK( testMatch( Path{{"foo"}},                "bar" ) == false );
  CHECK( testMatch( Path{{"no", "such", "node"}}, "a=c" ) == false );
  CHECK( testMatch( Path{{}},                     "a=c" ) == false );
  CHECK( testMatchMulti( Path{{"narf"}},          "42"  ) == true  );
  CHECK( testMatchMulti( Path{{"bar"}},           "xxx" ) == true  );
  CHECK( testMatchMulti( Path{{"bar"}},           "yyy" ) == true  );
  CHECK( testMatchMulti( Path{{"bar"}},           "zzz" ) == false );
  CHECK( testMatchMulti( Path{{"foo", "bar"}},    "xxx" ) == true  );
  CHECK( testMatchMulti( Path{{"foo", "bar"}},    "yyy" ) == true  );
  CHECK( testMatchMulti( Path{{"foo", "bar"}},    "zzz" ) == false );
}


TEST_CASE_FIXTURE(Fixture, "key comparison of absolute path")
{
  matchFunction = matchesKey;
  CHECK( testMatch( Path{{".", "PING", "PONG", "narf"}}, "fran"   ) == true  );
  CHECK( testMatch( Path{{".", "PING", "PONG", "narf"}}, "FRAN"   ) == false );
  CHECK( testMatch( Path{{".", "PING", "PONG", "narf"}}, "FRANxx" ) == false );
  CHECK( testMatch( Path{{".", "PONG", "narf"}},         "fran"   ) == false );
  CHECK( testMatch( Path{{".", "foo"}},                  "bar"    ) == true  );
  CHECK( testMatch( Path{{".", "foo"}},                  "BAR"    ) == false );
  CHECK( testMatch( Path{{".", "foo"}},                  "ba$"    ) == false );
  CHECK( testMatch( Path{{".", "no", "such", "node"}},   "bar"    ) == false );
  CHECK( testMatch( Path{{"."}},                         "bar"    ) == false );
  CHECK( testMatch( Path{{".", "array[]"}},              "^on.$"  ) == true );
  CHECK( testMatch( Path{{".", "array[]"}},              "^XYZ"   ) == false );
}


TEST_CASE_FIXTURE(Fixture, "key comparison of relative path")
{
  matchFunction = matchesKey;
  CHECK( testMatch( Path{{"narf"}},               "fran"   ) == true  );
  CHECK( testMatch( Path{{"narf"}},               "FRAN"   ) == false );
  CHECK( testMatch( Path{{"PONG", "narf"}},       "fran"   ) == true  );
  CHECK( testMatch( Path{{"PONG", "narf"}},       "FRAN"   ) == false );
  CHECK( testMatch( Path{{"PONG", "narf"}},       "franXX" ) == false );
  CHECK( testMatch( Path{{"PONG", "narf"}},       "FRANxx" ) == false );
  CHECK( testMatch( Path{{"foo"}},                "bar"    ) == true  );
  CHECK( testMatch( Path{{"foo"}},                "ba$"    ) == false );
  CHECK( testMatch( Path{{"no", "such", "node"}}, "bar"    ) == false );
  CHECK( testMatch( Path{{}},                     "bar"    ) == false );
  CHECK( testMatchMulti( Path{{"narf"}},          "fran"   ) == true  );
  CHECK( testMatchMulti( Path{{"narf"}},          "FRAN"   ) == false );
  CHECK( testMatchMulti( Path{{"foo"}},           "bar"    ) == true  );
  CHECK( testMatchMulti( Path{{"foo"}},           "BAR"    ) == false );
  CHECK( testMatchMulti( Path{{"foo"}},           "zzz"    ) == false );
  CHECK( testMatchMulti( Path{{"foo"}},           "ZZZ"    ) == false );
}


TEST_CASE_FIXTURE(Fixture, "getting all key values for an aboslute path")
{
  SUBCASE("existing key")
  {
    const auto out = allValues(AnnotatedLog{log_}, Path{{".", "foo", "bar"}});
    REQUIRE( out.size() == 1 );
    CHECK( out[0] == "a/c" );
  }
  SUBCASE("non-existing key")
  {
    const auto out = allValues(AnnotatedLog{log_}, Path{{".", "xxx", "yyy"}});
    CHECK( out.size() == 0 );
  }
}


TEST_CASE_FIXTURE(Fixture, "getting all key values for a relative path")
{
  SUBCASE("existing key")
  {
    auto out = allValues(AnnotatedLog{logMulti_}, Path{{"foo", "bar"}});
    REQUIRE( out.size() == 2 );
    std::sort( begin(out), end(out) );
    CHECK( out[0] == "xxx" );
    CHECK( out[1] == "yyy" );
  }
  SUBCASE("non-existing key")
  {
    const auto out = allValues(AnnotatedLog{log_}, Path{{"xxx", "yyy"}});
    CHECK( out.size() == 0 );
  }
}


TEST_CASE_FIXTURE(Fixture, "keys can only be taken out of key:value pair (not object/array)")
{
  auto out = allValues(AnnotatedLog{logMulti_}, Path{{"foo"}});
  REQUIRE( out.size() == 0 );
}


TEST_CASE_FIXTURE(Fixture, "relative paths leading to the same values are unified")
{
  const Log repeatedLog{ makeLog(R"({
                                "one": {
                                  "bar": "xx"
                                },
                                "two": {
                                  "bar": "xx"
                                }
                              })") };
  auto out = allValues(AnnotatedLog{repeatedLog}, Path{{"bar"}});
  REQUIRE( out.size() == 1 );
  CHECK( out[0] == "xx" );
}


TEST_CASE_FIXTURE(Fixture, "getting values for nodes")
{
  SUBCASE("relative path to key:value")
  {
    auto out = allNodeValues(AnnotatedLog{logMulti_}, Path::parse("bar"));
    std::sort( begin(out), end(out) );
    REQUIRE( out.size() == 2 );
    CHECK( out[0] == "xxx" );
    CHECK( out[1] == "yyy" );
  }
  SUBCASE("relative path to nested structure")
  {
    auto out = allNodeValues(AnnotatedLog{logMulti_}, Path::parse("foo"));
    std::sort( begin(out), end(out) );
    REQUIRE( out.size() == 3 );
    CHECK( out[0] == R"([{"one":1},{"two":2}])" );
    CHECK( out[1] == R"({"bar":"xxx"})" );
    CHECK( out[2] == R"({"bar":"yyy"})" );
  }
  SUBCASE("absolute path to key:value")
  {
    auto out = allNodeValues(AnnotatedLog{log_}, Path::parse(".foo.bar"));
    REQUIRE( out.size() == 1 );
    CHECK( out[0] == "a/c" );
  }
  SUBCASE("absolute path to nested structure")
  {
    auto out = allNodeValues(AnnotatedLog{log_}, Path::parse(".foo"));
    REQUIRE( out.size() == 1 );
    CHECK( out[0] == R"({"bar":"a/c"})" );
  }
}


namespace
{
template<typename LogType>
void testMatchingKeyByString(Fixture const& fixture)
{
  CHECK( matchesAnyKey( LogType{fixture.log_}, "" ) == true );
  CHECK( matchesAnyKey( LogType{fixture.log_}, "x" ) == false );
  CHECK( matchesAnyKey( LogType{fixture.log_}, "1" ) == false );
  CHECK( matchesAnyKey( LogType{fixture.log_}, "a" ) == true );
  CHECK( matchesAnyKey( LogType{fixture.log_}, "f" ) == true );
  CHECK( matchesAnyKey( LogType{fixture.log_}, "fo" ) == true );
  CHECK( matchesAnyKey( LogType{fixture.log_}, "foo" ) == true );
  CHECK( matchesAnyKey( LogType{fixture.log_}, "foO" ) == false );
  CHECK( matchesAnyKey( LogType{fixture.log_}, "oo" ) == true );
  CHECK( matchesAnyKey( LogType{fixture.log_}, "array" ) == true );
  CHECK( matchesAnyKey( LogType{fixture.log_}, "a_c" ) == false );
  CHECK( matchesAnyKey( LogType{fixture.logMulti_}, "1" ) == false );
  CHECK( matchesAnyKey( LogType{fixture.logMulti_}, "aaa" ) == false);
  CHECK( matchesAnyKey( LogType{fixture.logMulti_}, "an" ) == true );
}
}

TEST_CASE_FIXTURE(Fixture, "matches any key")
{
  SUBCASE("match string")
  {
    testMatchingKeyByString<Log>(*this);
    testMatchingKeyByString<AnnotatedLog>(*this);
  }
  SUBCASE("match regex")
  {
    // TODO...
  }
}


namespace
{
template<typename LogType>
void testMatchingValueByString(Fixture const& fixture)
{
  CHECK( matchesAnyValue( LogType{fixture.log_}, "" ) == true );
  CHECK( matchesAnyValue( LogType{fixture.log_}, "x" ) == false );
  CHECK( matchesAnyValue( LogType{fixture.log_}, "1" ) == true );
  CHECK( matchesAnyValue( LogType{fixture.log_}, "a" ) == true );
  CHECK( matchesAnyValue( LogType{fixture.log_}, "a_" ) == true );
  CHECK( matchesAnyValue( LogType{fixture.log_}, "a_c" ) == true );
  CHECK( matchesAnyValue( LogType{fixture.log_}, "a_C" ) == false );
  CHECK( matchesAnyValue( LogType{fixture.log_}, "array" ) == false );
  CHECK( matchesAnyValue( LogType{fixture.logMulti_}, "4" ) == true );
  CHECK( matchesAnyValue( LogType{fixture.logMulti_}, "2" ) == true );
  CHECK( matchesAnyValue( LogType{fixture.logMulti_}, "3" ) == false );
}
}

TEST_CASE_FIXTURE(Fixture, "matches any value")
{
  SUBCASE("match string")
  {
    testMatchingValueByString<Log>(*this);
    testMatchingValueByString<AnnotatedLog>(*this);
  }
  SUBCASE("match regex")
  {
    // TODO...
  }
}


namespace
{
template<typename LogType>
void testMatchingKeyValueByString(Fixture const& fixture)
{
  CHECK( matchesAnyKeyValue( LogType{fixture.log_}, "", "" ) == true );
  CHECK( matchesAnyKeyValue( LogType{fixture.log_}, "fran", "x" ) == false );
  CHECK( matchesAnyKeyValue( LogType{fixture.log_}, "fra", "a_" ) == true );
  CHECK( matchesAnyKeyValue( LogType{fixture.log_}, "one", "1" ) == true );
  CHECK( matchesAnyKeyValue( LogType{fixture.log_}, "two", "1" ) == false );
  CHECK( matchesAnyKeyValue( LogType{fixture.log_}, "one", "2" ) == false );
  CHECK( matchesAnyKeyValue( LogType{fixture.log_}, "a", "a" ) == true );
  CHECK( matchesAnyKeyValue( LogType{fixture.log_}, "narf", "a_" ) == false );
  CHECK( matchesAnyKeyValue( LogType{fixture.log_}, "", "a_c" ) == true );
  CHECK( matchesAnyKeyValue( LogType{fixture.log_}, "", "a_C" ) == false );
  CHECK( matchesAnyKeyValue( LogType{fixture.log_}, "", "array" ) == false );
  CHECK( matchesAnyKeyValue( LogType{fixture.logMulti_}, "rf", "4" ) == true );
  CHECK( matchesAnyKeyValue( LogType{fixture.logMulti_}, "na", "2" ) == true );
  CHECK( matchesAnyKeyValue( LogType{fixture.logMulti_}, "narf", "3" ) == false );
}
}

TEST_CASE_FIXTURE(Fixture, "matches any key=value pairs")
{
  SUBCASE("match string")
  {
    testMatchingKeyValueByString<Log>(*this);
    testMatchingKeyValueByString<AnnotatedLog>(*this);
  }
  SUBCASE("match regex")
  {
    // TODO...
  }
}


TEST_CASE_FIXTURE(Fixture, "example test cases for handling array elements and indexes")
{
  const auto log = makeLog(R"({
                                "four": {
                                  "foo": [ 77, 88 ]
                                },
                                "five": {
                                  "foo": [
                                    { "one": 77 },
                                    { "two": 88 },
                                    { "three": 99 }
                                  ]
                                }
                              })");

  SUBCASE("match value")
  {
    matchFunction = matchesValue;
    CHECK( testMatch(Path::parse(".four.foo"), "77", log) == false );
    CHECK( testMatch(Path::parse(".four.foo[]"), "77", log) == true );
    CHECK( testMatch(Path::parse(".five.foo.one"), "77", log) == false );
    CHECK( testMatch(Path::parse(".five.foo[].one"), "77", log) == true );
  }
  SUBCASE("match key")
  {
    matchFunction = matchesKey;
    CHECK( testMatch(Path::parse(".four.foo"), "1", log) == false );
    CHECK( testMatch(Path::parse(".four.foo[]"), "1", log) == false );
    CHECK( testMatch(Path::parse(".five.foo"), "one", log) == false );
    CHECK( testMatch(Path::parse(".five.foo[]"), "one", log) == true );
  }
  SUBCASE("check if key exists in array")
  {
    matchFunction = matchesKey;
    CHECK( testMatch(Path::parse("foo[2]"), "", log) == true );
    CHECK( testMatch(Path::parse("foo[3]"), "", log) == false );
  }
}


}
}
