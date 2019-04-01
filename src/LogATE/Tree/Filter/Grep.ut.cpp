#include <doctest/doctest.h>
#include "LogATE/Tree/Filter/Grep.hpp"

using LogATE::Log;
using LogATE::json2log;
using LogATE::Tree::Path;
using LogATE::Tree::Filter::Grep;

namespace
{

TEST_SUITE("Tree::Filter::Grep")
{

struct Fixture
{
  auto testMatch(Path const& path, std::string const& re, Log const& log) const
  {
    Grep g{Grep::Name{"foo"}, path, re, compare_, case_};
    REQUIRE( g.logs().withLock()->empty() );
    g.insert(log);
    return g.logs().withLock()->size();
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
                                }
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
                                }
                              })") };
  Grep::Compare compare_;
  Grep::Case case_;
};


TEST_CASE_FIXTURE(Fixture, "converting all basic types for value comparison")
{
  SUBCASE("string")
  {
    Grep g{Grep::Name{"foo"}, Path{{".", "val"}}, "ax*", Grep::Compare::Value, Grep::Case::Sensitive};
    g.insert( json2log(R"({ "val": "axxx" })") );
    CHECK( g.logs().withLock()->size() == 1 );
  }
  SUBCASE("int")
  {
    Grep g{Grep::Name{"foo"}, Path{{".", "val"}}, "42", Grep::Compare::Value, Grep::Case::Sensitive};
    g.insert( json2log(R"({ "val": 42 })") );
    CHECK( g.logs().withLock()->size() == 1 );
  }
  SUBCASE("float")
  {
    Grep g{Grep::Name{"foo"}, Path{{".", "val"}}, "4.0", Grep::Compare::Value, Grep::Case::Sensitive};
    g.insert( json2log(R"({ "val": 4.0 })") );
    CHECK( g.logs().withLock()->size() == 1 );
  }
  SUBCASE("bool")
  {
    Grep g{Grep::Name{"foo"}, Path{{".", "val"}}, "true", Grep::Compare::Value, Grep::Case::Sensitive};
    g.insert( json2log(R"({ "val": true })") );
    CHECK( g.logs().withLock()->size() == 1 );
  }
  SUBCASE("regex matches in the middle as well")
  {
    Grep g{Grep::Name{"foo"}, Path{{".", "val"}}, "xxx", Grep::Compare::Value, Grep::Case::Sensitive};
    g.insert( json2log(R"({ "val": "foo-xxx-bar" })") );
    CHECK( g.logs().withLock()->size() == 1 );
  }
}


TEST_CASE_FIXTURE(Fixture, "case-sensitive value comparison of absolute path with regexs")
{
  compare_ = Grep::Compare::Value;
  case_    = Grep::Case::Sensitive;
  CHECK( testMatch( Path{{".", "foo", "bar"}},         "a.*c" ) == 1 );
  CHECK( testMatch( Path{{".", "FOO", "BAR"}},         "a.*c" ) == 0 );
  CHECK( testMatch( Path{{".", "foo"}},                "b.*r" ) == 0 );
  CHECK( testMatch( Path{{".", "no", "such", "node"}}, "a.*c" ) == 0 );
  CHECK( testMatch( Path{{"."}},                       "a.*c" ) == 0 );
}

TEST_CASE_FIXTURE(Fixture, "case-sensitive value comparison of absolute path")
{
  compare_ = Grep::Compare::Value;
  case_    = Grep::Case::Sensitive;
  CHECK( testMatch( Path{{".", "foo", "bar"}},         "a/c" ) == 1 );
  CHECK( testMatch( Path{{".", "foo", "bar"}},         "A/C" ) == 0 );
  CHECK( testMatch( Path{{".", "FOO", "BAR"}},         "a/c" ) == 0 );
  CHECK( testMatch( Path{{".", "foo"}},                "bar" ) == 0 );
  CHECK( testMatch( Path{{".", "no", "such", "node"}}, "a=c" ) == 0 );
  CHECK( testMatch( Path{{"."}},                       "a=c" ) == 0 );
}

TEST_CASE_FIXTURE(Fixture, "case-sensitive value comparison of relative path")
{
  compare_ = Grep::Compare::Value;
  case_    = Grep::Case::Sensitive;
  CHECK( testMatch( Path{{"fran"}},               "a_c" ) == 1 );
  CHECK( testMatch( Path{{"fran"}},               "A_C" ) == 0 );
  CHECK( testMatch( Path{{"narf", "fran"}},       "a_c" ) == 1 );
  CHECK( testMatch( Path{{"narf", "fran"}},       "A_C" ) == 0 );
  CHECK( testMatch( Path{{"NARF", "FRAN"}},       "a_c" ) == 0 );
  CHECK( testMatch( Path{{"ping", "fran"}},       "a_c" ) == 0 );
  CHECK( testMatch( Path{{"foo"}},                "bar" ) == 0 );
  CHECK( testMatch( Path{{"no", "such", "node"}}, "a=c" ) == 0 );
  CHECK( testMatch( Path{{}},                     "a=c" ) == 0 );
  CHECK( testMatchMulti( Path{{"narf"}},          "42"  ) == 1 );
  CHECK( testMatchMulti( Path{{"bar"}},           "xxx" ) == 1 );
  CHECK( testMatchMulti( Path{{"bar"}},           "yyy" ) == 1 );
  CHECK( testMatchMulti( Path{{"bar"}},           "zzz" ) == 0 );
  CHECK( testMatchMulti( Path{{"foo", "bar"}},    "xxx" ) == 1 );
  CHECK( testMatchMulti( Path{{"foo", "bar"}},    "yyy" ) == 1 );
  CHECK( testMatchMulti( Path{{"foo", "bar"}},    "zzz" ) == 0 );
}


TEST_CASE_FIXTURE(Fixture, "case-sensitive key comparison of absolute path")
{
  compare_ = Grep::Compare::Key;
  case_    = Grep::Case::Sensitive;
  CHECK( testMatch( Path{{".", "PING", "PONG", "narf"}}, "fran"   ) == 1 );
  CHECK( testMatch( Path{{".", "PING", "PONG", "narf"}}, "FRAN"   ) == 0 );
  CHECK( testMatch( Path{{".", "PING", "PONG", "narf"}}, "FRANxx" ) == 0 );
  CHECK( testMatch( Path{{".", "PONG", "narf"}},         "fran"   ) == 0 );
  CHECK( testMatch( Path{{".", "foo"}},                "bar"    ) == 1 );
  CHECK( testMatch( Path{{".", "foo"}},                "BAR"    ) == 0 );
  CHECK( testMatch( Path{{".", "foo"}},                "ba$"    ) == 0 );
  CHECK( testMatch( Path{{".", "no", "such", "node"}}, "bar"    ) == 0 );
  CHECK( testMatch( Path{{"."}},                       "bar"    ) == 0 );
}

TEST_CASE_FIXTURE(Fixture, "case-sensitive key comparison of relative path")
{
  compare_ = Grep::Compare::Key;
  case_    = Grep::Case::Sensitive;
  CHECK( testMatch( Path{{"narf"}},               "fran"   ) == 1 );
  CHECK( testMatch( Path{{"narf"}},               "FRAN"   ) == 0 );
  CHECK( testMatch( Path{{"PONG", "narf"}},       "fran"   ) == 1 );
  CHECK( testMatch( Path{{"PONG", "narf"}},       "FRAN"   ) == 0 );
  CHECK( testMatch( Path{{"PONG", "narf"}},       "franXX" ) == 0 );
  CHECK( testMatch( Path{{"PONG", "narf"}},       "FRANxx" ) == 0 );
  CHECK( testMatch( Path{{"foo"}},                "bar"    ) == 1 );
  CHECK( testMatch( Path{{"foo"}},                "ba$"    ) == 0 );
  CHECK( testMatch( Path{{"no", "such", "node"}}, "bar"    ) == 0 );
  CHECK( testMatch( Path{{}},                     "bar"    ) == 0 );
  CHECK( testMatchMulti( Path{{"narf"}},          "fran"   ) == 1 );
  CHECK( testMatchMulti( Path{{"narf"}},          "FRAN"   ) == 0 );
  CHECK( testMatchMulti( Path{{"foo"}},           "bar" ) == 1 );
  CHECK( testMatchMulti( Path{{"foo"}},           "BAR" ) == 0 );
  CHECK( testMatchMulti( Path{{"foo"}},           "zzz" ) == 0 );
  CHECK( testMatchMulti( Path{{"foo"}},           "ZZZ" ) == 0 );
}




TEST_CASE_FIXTURE(Fixture, "case-insensitive value comparison of absolute path")
{
  compare_ = Grep::Compare::Value;
  case_    = Grep::Case::Insensitive;
  CHECK( testMatch( Path{{".", "foo", "bar"}},         "a/c" ) == 1 );
  CHECK( testMatch( Path{{".", "foo", "bar"}},         "A/C" ) == 1 );
  CHECK( testMatch( Path{{".", "FOO", "BAR"}},         "a/c" ) == 0 );
  CHECK( testMatch( Path{{".", "foo"}},                "bar" ) == 0 );
  CHECK( testMatch( Path{{".", "no", "such", "node"}}, "a=c" ) == 0 );
  CHECK( testMatch( Path{{"."}},                       "a=c" ) == 0 );
}

TEST_CASE_FIXTURE(Fixture, "case-insensitive value comparison of relative path")
{
  compare_ = Grep::Compare::Value;
  case_    = Grep::Case::Insensitive;
  CHECK( testMatch( Path{{"fran"}},               "a_c" ) == 1 );
  CHECK( testMatch( Path{{"fran"}},               "A_C" ) == 1 );
  CHECK( testMatch( Path{{"narf", "fran"}},       "a_c" ) == 1 );
  CHECK( testMatch( Path{{"narf", "fran"}},       "A_C" ) == 1 );
  CHECK( testMatch( Path{{"NARF", "FRAN"}},       "a_c" ) == 0 );
  CHECK( testMatch( Path{{"ping", "fran"}},       "a_c" ) == 0 );
  CHECK( testMatch( Path{{"foo"}},                "bar" ) == 0 );
  CHECK( testMatch( Path{{"no", "such", "node"}}, "a=c" ) == 0 );
  CHECK( testMatch( Path{{}},                     "a=c" ) == 0 );
  CHECK( testMatchMulti( Path{{"narf"}},          "42"  ) == 1 );
  CHECK( testMatchMulti( Path{{"bar"}},           "xxx" ) == 1 );
  CHECK( testMatchMulti( Path{{"bar"}},           "XXX" ) == 1 );
  CHECK( testMatchMulti( Path{{"bar"}},           "yyy" ) == 1 );
  CHECK( testMatchMulti( Path{{"bar"}},           "YYY" ) == 1 );
  CHECK( testMatchMulti( Path{{"bar"}},           "zzz" ) == 0 );
  CHECK( testMatchMulti( Path{{"bar"}},           "ZZZ" ) == 0 );
  CHECK( testMatchMulti( Path{{"foo", "bar"}},    "xxx" ) == 1 );
  CHECK( testMatchMulti( Path{{"foo", "bar"}},    "XXX" ) == 1 );
  CHECK( testMatchMulti( Path{{"foo", "bar"}},    "yyy" ) == 1 );
  CHECK( testMatchMulti( Path{{"foo", "bar"}},    "YYY" ) == 1 );
  CHECK( testMatchMulti( Path{{"foo", "bar"}},    "zzz" ) == 0 );
}


TEST_CASE_FIXTURE(Fixture, "case-insensitive key comparison of absolute path")
{
  compare_ = Grep::Compare::Key;
  case_    = Grep::Case::Insensitive;
  CHECK( testMatch( Path{{".", "PING", "PONG", "narf"}}, "fran"   ) == 1 );
  CHECK( testMatch( Path{{".", "PING", "PONG", "narf"}}, "FRAN"   ) == 1 );
  CHECK( testMatch( Path{{".", "PING", "PONG", "narf"}}, "FRANxx" ) == 0 );
  CHECK( testMatch( Path{{".", "PONG", "narf"}},         "fran"   ) == 0 );
  CHECK( testMatch( Path{{".", "foo"}},                "bar"    ) == 1 );
  CHECK( testMatch( Path{{".", "foo"}},                "BAR"    ) == 1 );
  CHECK( testMatch( Path{{".", "foo"}},                "ba$"    ) == 0 );
  CHECK( testMatch( Path{{".", "no", "such", "node"}}, "bar"    ) == 0 );
  CHECK( testMatch( Path{{"."}},                       "bar"    ) == 0 );
}

TEST_CASE_FIXTURE(Fixture, "case-insensitive key comparison of relative path")
{
  compare_ = Grep::Compare::Key;
  case_    = Grep::Case::Insensitive;
  CHECK( testMatch( Path{{"narf"}},               "fran"   ) == 1 );
  CHECK( testMatch( Path{{"narf"}},               "FRAN"   ) == 1 );
  CHECK( testMatch( Path{{"PONG", "narf"}},       "fran"   ) == 1 );
  CHECK( testMatch( Path{{"PONG", "narf"}},       "FRAN"   ) == 1 );
  CHECK( testMatch( Path{{"PONG", "narf"}},       "franXX" ) == 0 );
  CHECK( testMatch( Path{{"PONG", "narf"}},       "FRANxx" ) == 0 );
  CHECK( testMatch( Path{{"foo"}},                "bar"    ) == 1 );
  CHECK( testMatch( Path{{"foo"}},                "ba$"    ) == 0 );
  CHECK( testMatch( Path{{"no", "such", "node"}}, "bar"    ) == 0 );
  CHECK( testMatch( Path{{}},                     "bar"    ) == 0 );
  CHECK( testMatchMulti( Path{{"narf"}},          "fran"   ) == 1 );
  CHECK( testMatchMulti( Path{{"narf"}},          "FRAN"   ) == 1 );
  CHECK( testMatchMulti( Path{{"foo"}},           "bar" ) == 1 );
  CHECK( testMatchMulti( Path{{"foo"}},           "BAR" ) == 1 );
  CHECK( testMatchMulti( Path{{"foo"}},           "zzz" ) == 0 );
  CHECK( testMatchMulti( Path{{"foo"}},           "ZZZ" ) == 0 );
}


// TODO: test for arrays as well!
// TODO: test for case where relative path is repeated in different subnodes, but some of them do not match while others do

}

}
