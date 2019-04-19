#include <doctest/doctest.h>
#include "LogATE/Tree/Filter/detail/matchesLog.hpp"

using LogATE::Log;
using LogATE::json2log;
using LogATE::Tree::Path;
using LogATE::Tree::Filter::detail::matchesKey;
using LogATE::Tree::Filter::detail::matchesValue;
using LogATE::Tree::Filter::detail::allValues;
using LogATE::Tree::Filter::detail::g_defaultRegexType;

namespace
{
TEST_SUITE("Tree::Filter::detail::jsonSearch")
{

struct Fixture
{
  auto testMatch(Path const& path, std::string const& re, Log const& log) const
  {
    const std::regex reg{re, g_defaultRegexType};
    REQUIRE(matchFunction);
    return (*matchFunction)(log, path, reg);
  }

  auto testMatch(Path const& path, std::string const& re) const
  {
    return testMatch(path, re, log_);
  }
  auto testMatchMulti(Path const& path, std::string const& re) const
  {
    return testMatch(path, re, logMulti_);
  }

  const Log log_{ json2log(R"({
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
                              })") };
  const Log logMulti_{ json2log(R"({
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
                              })") };
  bool (*matchFunction)(Log const&, Path const&, std::regex const&) = nullptr;
};


TEST_CASE_FIXTURE(Fixture, "converting all basic types for value comparison")
{
  matchFunction = matchesValue;
  SUBCASE("string")
  {
    const auto log = json2log(R"({ "val": "axxx" })");
    CHECK( testMatch(Path{{".", "val"}}, "ax*", log) );
  }
  SUBCASE("int")
  {
    const auto log = json2log(R"({ "val": 42 })");
    CHECK( testMatch(Path{{".", "val"}}, "^42$", log) );
  }
  SUBCASE("float")
  {
    const auto log = json2log(R"({ "val": 4.0 })");
    CHECK( testMatch(Path{{".", "val"}}, "^4.0*", log) );
  }
  SUBCASE("bool")
  {
    const auto log = json2log(R"({ "val": true })");
    CHECK( testMatch(Path{{".", "val"}}, "true", log) );
  }
  SUBCASE("regex matches in the middle as well")
  {
    const auto log = json2log(R"({ "val": "foo-xxx-bar" })");
    CHECK( testMatch(Path{{".", "val"}}, "xxx", log) );
  }
}

TEST_CASE_FIXTURE(Fixture, "value comparison of absolute path with regexs")
{
  matchFunction = matchesValue;
  CHECK( testMatch( Path{{".", "foo", "bar"}},         "a.*c" ) == true  );
  CHECK( testMatch( Path{{".", "FOO", "BAR"}},         "a.*c" ) == false );
  CHECK( testMatch( Path{{".", "foo"}},                "b.*r" ) == false );
  CHECK( testMatch( Path{{".", "no", "such", "node"}}, "a.*c" ) == false );
  CHECK( testMatch( Path{{"."}},                       "a.*c" ) == false );
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
    const auto out = allValues(log_, Path{{".", "foo", "bar"}});
    REQUIRE( out.size() == 1 );
    CHECK( out[0] == "a/c" );
  }
  SUBCASE("non-existing key")
  {
    const auto out = allValues(log_, Path{{".", "xxx", "yyy"}});
    CHECK( out.size() == 0 );
  }
}

TEST_CASE_FIXTURE(Fixture, "getting all key values for a relative path")
{
  SUBCASE("existing key")
  {
    auto out = allValues(logMulti_, Path{{"foo", "bar"}});
    REQUIRE( out.size() == 2 );
    std::sort( begin(out), end(out) );
    CHECK( out[0] == "xxx" );
    CHECK( out[1] == "yyy" );
  }
  SUBCASE("non-existing key")
  {
    const auto out = allValues(log_, Path{{"xxx", "yyy"}});
    CHECK( out.size() == 0 );
  }
}

TEST_CASE_FIXTURE(Fixture, "keys can only be taken out of key:value pair (not object/array)")
{
  auto out = allValues(logMulti_, Path{{"foo"}});
  REQUIRE( out.size() == 0 );
}

TEST_CASE_FIXTURE(Fixture, "relative paths leading to the same values are unified")
{
  const Log repeatedLog{ json2log(R"({
                                "one": {
                                  "bar": "xx"
                                },
                                "two": {
                                  "bar": "xx"
                                }
                              })") };
  auto out = allValues(repeatedLog, Path{{"bar"}});
  REQUIRE( out.size() == 1 );
  CHECK( out[0] == "xx" );
}

}
}
