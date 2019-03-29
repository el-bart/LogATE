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
  auto testMatch(Path const& path, std::string const& re) const
  {
    Grep g{Grep::Name{"foo"}, path, re, compare_, case_};
    REQUIRE( g.logs().withLock()->empty() );
    g.insert(log_);
    return g.logs().withLock()->size();
  }

  const Log log_{ json2log(R"({
                                "PING": {
                                  "PONG": {
                                    "narf": {
                                      "fran": "a+c"
                                    }
                                  }
                                },
                                "foo": {
                                  "bar": "a/c"
                                }
                              })") };
  Grep::Compare compare_;
  Grep::Case case_;
};


TEST_CASE_FIXTURE(Fixture, "case-sensitive value comparison of absolute path")
{
  compare_ = Grep::Compare::Value;
  case_    = Grep::Case::Sensitive;
  CHECK( testMatch( Path{{".", "foo", "bar"}},         "a/c" ) == 1 );
  CHECK( testMatch( Path{{".", "FOO", "BAR"}},         "a/c" ) == 0 );
  CHECK( testMatch( Path{{".", "foo"}},                "bar" ) == 0 );
  CHECK( testMatch( Path{{".", "no", "such", "node"}}, "a+c" ) == 0 );
}

/*
TEST_CASE_FIXTURE(Fixture, "case-sensitive value comparison of relative path")
{
  Grep g{name_, Path{{"foo", "bar"}}, re_, Grep::Compare::Value, Grep::Case::Sensitive};
  (void)g;
}


TEST_CASE_FIXTURE(Fixture, "case-sensitive key comparison of absolute path")
{
  Grep g{name_, Path{{".", "foo", "bar"}}, re_, Grep::Compare::Key, Grep::Case::Sensitive};
  (void)g;
}

TEST_CASE_FIXTURE(Fixture, "case-sensitive key comparison of relative path")
{
  Grep g{name_, Path{{"foo", "bar"}}, re_, Grep::Compare::Key, Grep::Case::Sensitive};
  (void)g;
}




TEST_CASE_FIXTURE(Fixture, "case-insensitive value comparison of absolute path")
{
  Grep g{name_, Path{{".", "foo", "bar"}}, re_, Grep::Compare::Value, Grep::Case::Insensitive};
  (void)g;
}

TEST_CASE_FIXTURE(Fixture, "case-insensitive value comparison of relative path")
{
  Grep g{name_, Path{{"foo", "bar"}}, re_, Grep::Compare::Value, Grep::Case::Insensitive};
  (void)g;
}


TEST_CASE_FIXTURE(Fixture, "case-insensitive key comparison of absolute path")
{
  Grep g{name_, Path{{".", "foo", "bar"}}, re_, Grep::Compare::Key, Grep::Case::Insensitive};
  (void)g;
}

TEST_CASE_FIXTURE(Fixture, "case-insensitive key comparison of relative path")
{
  Grep g{name_, Path{{"foo", "bar"}}, re_, Grep::Compare::Key, Grep::Case::Insensitive};
  (void)g;
}
*/

}

}
