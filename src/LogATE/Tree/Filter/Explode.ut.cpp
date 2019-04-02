#include <doctest/doctest.h>
#include "LogATE/Tree/Filter/Explode.hpp"

using LogATE::Log;
using LogATE::json2log;
using LogATE::Tree::Path;
using LogATE::Tree::Filter::Explode;

namespace
{
TEST_SUITE("Tree::Filter::Explode")
{

struct Fixture
{
  auto testMatch(Path const& path, Log const& log) const
  {
    Explode e{Explode::Name{"foo"}, path};
    REQUIRE( e.logs().withLock()->empty() );
    e.insert(log);
    return e.logs().withLock()->size();
  }

  auto testMatch(Path const& path) const
  {
    return testMatch(path, log_);
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
};


TEST_CASE_FIXTURE(Fixture, "xxx")
{
}

}
}
