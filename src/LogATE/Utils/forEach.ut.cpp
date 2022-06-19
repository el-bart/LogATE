#include <doctest/doctest.h>
#include "LogATE/Utils/forEach.hpp"
#include <optional>
#include <set>

using LogATE::Utils::forEach;

namespace
{
TEST_SUITE("Tree::Utils::forEach")
{

struct Recorder
{
  bool operator()(nlohmann::json const& value)
  {
    ++callCount_;
    values_.insert(&value);
    if( stopAfter_ && *stopAfter_ <= callCount_ )
      return false;
    return true;
  }

  std::set<nlohmann::json const*> values_;
  std::optional<unsigned> stopAfter_{};
  unsigned callCount_{0};
};


struct Fixture
{
  Fixture()
  {
    assert( nested_.is_object() && "nlohmann::json{ nlohmann::json{...} } created 1-element array!" );
  }

  Recorder rec_;
  const nlohmann::json nested_ =
                            R"({
                                "meh": {
                                  "foo": [
                                      { "bar": [ 1, 2, 3 ] },
                                      { "bar": [ 42, 13 ] }
                                    ]
                                  },
                                  "number": 42,
                                  "fp": 3.14,
                                  "string": "trust no1",
                                   "bool": false
                                })"_json;
};


TEST_CASE_FIXTURE(Fixture, "null json")
{
  const nlohmann::json null_{};
  CHECK( forEach(null_, rec_) == true );
  CHECK( rec_.callCount_ == 0u );
  REQUIRE( rec_.values_.size() == 0u );
}


TEST_CASE_FIXTURE(Fixture, "example json")
{
  CHECK( forEach(nested_, rec_) == true );
  CHECK( rec_.callCount_ == 15u );
  REQUIRE( rec_.values_.size() == 15u );
}


TEST_CASE_FIXTURE(Fixture, "prepature termination on user request")
{
  rec_.stopAfter_ = 8;
  CHECK( forEach(nested_, rec_) == false );
  CHECK( rec_.callCount_ == 8u );
  REQUIRE( rec_.values_.size() == 8u );
}

}
}
