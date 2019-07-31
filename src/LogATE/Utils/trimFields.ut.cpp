#include <doctest/doctest.h>
#include "LogATE/Utils/trimFields.hpp"
#include "LogATE/TestHelpers.ut.hpp"

using LogATE::Utils::trimFields;
using LogATE::Log;
using LogATE::AnnotatedLog;
using LogATE::makeLog;
using LogATE::Tree::Path;
using LogATE::Tree::Node;

namespace
{
TEST_SUITE("Tree::Utils::trimFields")
{

struct Fixture
{
  const Log log_{ makeLog(R"({
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
  const Log logMulti_{ makeLog(R"({
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
};


TEST_CASE_FIXTURE(Fixture, "no action taken on empty list")
{
  const auto tf = Node::TrimFields{};
  CHECK( trimFields(log_, tf).sequenceNumber() == log_.sequenceNumber() );
  CHECK( trimFields(log_, tf).json().dump(2) == log_.json().dump(2) );
}


TEST_CASE_FIXTURE(Fixture, "trimming empty path does nothing")
{
  const auto tf = Node::TrimFields{ Path{} };
  CHECK( trimFields(log_, tf).sequenceNumber() == log_.sequenceNumber() );
  CHECK( trimFields(log_, tf).json().dump(2) == log_.json().dump(2) );
}


TEST_CASE_FIXTURE(Fixture, "trimming non-existent path does nothing")
{
  const auto tf = Node::TrimFields{ Path::parse(".no.such.path"), Path::parse("does.not.exist") };
  CHECK( trimFields(log_, tf).sequenceNumber() == log_.sequenceNumber() );
  CHECK( trimFields(log_, tf).json().dump(2) == log_.json().dump(2) );
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
  SUBCASE("leaf")
  {
    const auto tf = Node::TrimFields{ Path::parse("fran") };
    auto out = logMulti_.json();
    out["two"]["PING"]["PONG"]["narf"].erase("fran");
    CHECK( trimFields(logMulti_, tf).json().dump(2) == out.dump(2) );
  }
  SUBCASE("node")
  {
    const auto tf = Node::TrimFields{ Path::parse("three") };
    auto out = logMulti_.json();
    out.erase("three");
    CHECK( trimFields(logMulti_, tf).json().dump(2) == out.dump(2) );
  }
  SUBCASE("node with longer path")
  {
    const auto tf = Node::TrimFields{ Path::parse("three.foo") };
    auto out = logMulti_.json();
    out["three"].erase("foo");
    CHECK( trimFields(logMulti_, tf).json().dump(2) == out.dump(2) );
  }
}


TEST_CASE_FIXTURE(Fixture, "trimming multi-match paths")
{
  const auto tf = Node::TrimFields{ Path::parse("foo.bar") };
  auto out = logMulti_.json();
  out["three"]["foo"].erase("bar");
  out["four"]["foo"].erase("bar");
  CHECK( trimFields(logMulti_, tf).json().dump(2) == out.dump(2) );
}


TEST_CASE_FIXTURE(Fixture, "trimming multiple different paths")
{
  const auto tf = Node::TrimFields{ Path::parse(".foo.bar"), Path::parse("PONG") };
  auto out = log_.json();
  out["foo"].erase("bar");
  out["PING"].erase("PONG");
  CHECK( trimFields(log_, tf).json().dump(2) == out.dump(2) );
}

}
}
