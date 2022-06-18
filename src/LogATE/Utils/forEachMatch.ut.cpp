#include <doctest/doctest.h>
#include "LogATE/Utils/forEachMatch.hpp"
#include <limits>

using LogATE::Utils::forEachMatch;
using LogATE::Tree::Path;

namespace
{
TEST_SUITE("Tree::Utils::forEachMatch")
{

struct Fixture
{
  const nlohmann::json big_{ R"({
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
                              })" };
};


TEST_CASE_FIXTURE(Fixture, "absolute path")
{
  // TODO
}


TEST_CASE_FIXTURE(Fixture, "absolute path with arrays")
{
  // TODO
}


TEST_CASE_FIXTURE(Fixture, "absolute path with wildcard arrays")
{
  // TODO
}


TEST_CASE_FIXTURE(Fixture, "relative path")
{
  // TODO
}


TEST_CASE_FIXTURE(Fixture, "relative path with arrays")
{
  // TODO
}


TEST_CASE_FIXTURE(Fixture, "relative path with wildcard arrays")
{
  // TODO
}

}
}
