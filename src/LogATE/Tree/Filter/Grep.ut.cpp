#include <doctest/doctest.h>
#include "LogATE/Tree/Filter/Grep.hpp"
#include "LogATE/TestHelpers.ut.hpp"

using LogATE::Log;
using LogATE::AnnotatedLog;
using LogATE::makeKey;
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
    Grep g{workers_, Grep::Name{"foo"}, path, re, compare_, case_, search_, trim_};
    REQUIRE( g.logs().withLock()->empty() );
    g.insert( AnnotatedLog{log} );
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
  Grep::Compare compare_;
  Grep::Case case_;
  Grep::Search search_;
  Grep::Trim trim_;
  LogATE::Utils::WorkerThreadsShPtr workers_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>() };
};


TEST_CASE_FIXTURE(Fixture, "value comparison of absolute path with regexs")
{
  compare_ = Grep::Compare::Value;
  case_    = Grep::Case::Sensitive;
  search_  = Grep::Search::Regular;
  trim_    = Grep::Trim::False;
  CHECK( testMatch( Path{{".", "foo", "bar"}}, "a.*c" ) == 1 );
  CHECK( testMatch( Path{{".", "FOO", "BAR"}}, "a.*c" ) == 0 );
}


TEST_CASE_FIXTURE(Fixture, "value comparison of absolute path")
{
  compare_ = Grep::Compare::Value;
  case_    = Grep::Case::Sensitive;
  search_  = Grep::Search::Regular;
  trim_    = Grep::Trim::False;
  CHECK( testMatch( Path{{".", "foo", "bar"}}, "a/c" ) == 1 );
  CHECK( testMatch( Path{{".", "FOO", "BAR"}}, "a/c" ) == 0 );
}


TEST_CASE_FIXTURE(Fixture, "value comparison of relative path")
{
  compare_ = Grep::Compare::Value;
  case_    = Grep::Case::Sensitive;
  search_  = Grep::Search::Regular;
  trim_    = Grep::Trim::False;
  CHECK( testMatch( Path{{"fran"}},      "a_c" ) == 1 );
  CHECK( testMatch( Path{{"fran"}},      "A_C" ) == 0 );
  CHECK( testMatchMulti( Path{{"narf"}}, "42"  ) == 1 );
  CHECK( testMatchMulti( Path{{"bar"}},  "zzz" ) == 0 );
}


TEST_CASE_FIXTURE(Fixture, "key comparison of absolute path")
{
  compare_ = Grep::Compare::Key;
  case_    = Grep::Case::Sensitive;
  search_  = Grep::Search::Regular;
  trim_    = Grep::Trim::False;
  CHECK( testMatch( Path{{".", "foo"}}, "bar" ) == 1 );
  CHECK( testMatch( Path{{".", "foo"}}, "BAR" ) == 0 );
}


TEST_CASE_FIXTURE(Fixture, "key comparison of relative path")
{
  compare_ = Grep::Compare::Key;
  case_    = Grep::Case::Sensitive;
  search_  = Grep::Search::Regular;
  trim_    = Grep::Trim::False;
  CHECK( testMatch( Path{{"narf"}},      "fran"   ) == 1 );
  CHECK( testMatch( Path{{"narf"}},      "FRAN"   ) == 0 );
  CHECK( testMatchMulti( Path{{"narf"}}, "fran"   ) == 1 );
  CHECK( testMatchMulti( Path{{"narf"}}, "FRAN"   ) == 0 );
}


TEST_CASE_FIXTURE(Fixture, "case-insensitive search")
{
  compare_ = Grep::Compare::Value;
  case_    = Grep::Case::Insensitive;
  search_  = Grep::Search::Regular;
  trim_    = Grep::Trim::False;
  CHECK( testMatch( Path{{".", "foo", "bar"}}, "a/c" ) == 1 );
  CHECK( testMatch( Path{{".", "foo", "bar"}}, "A/C" ) == 1 );
  CHECK( testMatch( Path{{".", "FOO", "BAR"}}, "a/c" ) == 0 );
}


TEST_CASE_FIXTURE(Fixture, "inverse value search")
{
  compare_ = Grep::Compare::Value;
  case_    = Grep::Case::Sensitive;
  search_  = Grep::Search::Inverse;
  trim_    = Grep::Trim::False;
  CHECK( testMatch( Path{{".", "foo", "bar"}}, "a/c" ) == 0 );
  CHECK( testMatch( Path{{".", "foo", "bar"}}, "xxx" ) == 1 );
  CHECK( testMatch( Path{{".", "FOO", "BAR"}}, "a/c" ) == 1 );
}


TEST_CASE_FIXTURE(Fixture, "trimming fileds is optional")
{
  compare_ = Grep::Compare::Value;
  case_    = Grep::Case::Sensitive;
  search_  = Grep::Search::Regular;
  const auto path = Path::parse(".foo.bar");

  SUBCASE("trimming is not applied")
  {
    trim_ = Grep::Trim::False;
    const Grep g{workers_, Grep::Name{"foo"}, path, std::string{"a/b/c"}, compare_, case_, search_, trim_};
    CHECK( g.trimFields().empty() );
  }

  SUBCASE("trimming is applied")
  {
    trim_ = Grep::Trim::True;
    const Grep g{workers_, Grep::Name{"foo"}, path, std::string{"a/b/c"}, compare_, case_, search_, trim_};
    REQUIRE( g.trimFields().size() == 1 );
    CHECK( g.trimFields()[0] == path );
  }
}

}
}
