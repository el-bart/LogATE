#include <doctest/doctest.h>
#include "LogATE/Utils/trimFields.hpp"

using LogATE::Utils::trimFields;
using LogATE::Log;
using LogATE::AnnotatedLog;
using LogATE::Tree::Path;
using LogATE::Tree::Node;

namespace
{
TEST_SUITE("Tree::Utils::trimFields")
{

struct Fixture
{
  const Log log_{ R"({
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
  const Log logMulti_{ R"({
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
};


TEST_CASE_FIXTURE(Fixture, "no action taken on empty list")
{
  const auto tf = Node::TrimFields{};
  CHECK( trimFields(log_, tf).sequenceNumber() == log_.sequenceNumber() );
  CHECK( trimFields(log_, tf).json() == log_.json() );
}


TEST_CASE_FIXTURE(Fixture, "trimming non-existent path does nothing")
{
  const auto tf = Node::TrimFields{ Path::parse(".no.such.path"), Path::parse("does.not.exist") };
  CHECK( trimFields(log_, tf).sequenceNumber() == log_.sequenceNumber() );
  CHECK( trimFields(log_, tf).json() == log_.json() );
}


TEST_CASE_FIXTURE(Fixture, "trimming absolute path")
{
  SUBCASE("leaf")
  {
    const auto tf = Node::TrimFields{ Path::parse(".foo.bar") };
    auto out = log_.json();
    out["foo"].erase("bar");
    CHECK( trimFields(log_, tf).json().dump(2) == out.dump(2) );
  }
  SUBCASE("node")
  {
    const auto tf = Node::TrimFields{ Path::parse(".foo") };
    auto out = log_.json();
    out.erase("foo");
    CHECK( trimFields(log_, tf).json().dump(2) == out.dump(2) );
  }
}


TEST_CASE_FIXTURE(Fixture, "trimming relative path")
{
}


TEST_CASE_FIXTURE(Fixture, "trimming multiple paths")
{
}

}
}
