#include <doctest/doctest.h>
#include "LogATE/Tree/KeyExtractor.hpp"

using LogATE::Tree::KeyExtractor;
using LogATE::Tree::Path;

namespace
{
TEST_SUITE("Tree::KeyExtractor")
{

struct Fixture
{ };


TEST_CASE_FIXTURE(Fixture, "getting key in raw format")
{
  nlohmann::json j;
  j["iso"]     = "2025-07-02T20:06:13.123456789Z";
  j["unix"]    = 1751486773;
  j["unix_ms"] = 1751486773123;
  j["unix_us"] = 1751486773123456;
  j["unix_ns"] = 1751486773123456789;
  j["foo"]["bar"] = "sth";

  SUBCASE("raw format")
  {
    auto format = KeyExtractor::Format::Raw;
    SUBCASE("existing path (string)")
    {
      KeyExtractor const ke{ Path::parse(".foo.bar"), format };
      CHECK( ke.extract(j) ==  "sth" );
    }
    SUBCASE("existing path (numeric)")
    {
      KeyExtractor const ke{ Path::parse(".unix"), format };
      CHECK( ke.extract(j) ==  "1751486773" );
    }
    SUBCASE("non-existing path")
    {
      KeyExtractor const ke{ Path::parse(".not.there"), format };
      CHECK( ke.extract(j) == "<< key not found >>" );
    }
  }

  SUBCASE("ISO8601_ns")
  {
    auto format = KeyExtractor::Format::ISO8601_ns;
    SUBCASE("valid")
    {
      KeyExtractor const ke{ Path::parse(".iso"), format };
      CHECK( ke.extract(j) ==  "2025-07-02T20:06:13.123456789Z" );
    }
    SUBCASE("invalid")
    {
      KeyExtractor const ke{ Path::parse(".foo.bar"), format };
      CHECK( ke.extract(j) ==  "<< invalid key >>" );
    }
  }

  SUBCASE("UNIX")
  {
    auto format = KeyExtractor::Format::UNIX;
    SUBCASE("valid")
    {
      KeyExtractor const ke{ Path::parse(".unix"), format };
      CHECK( ke.extract(j) ==  "2025-07-02T20:06:13.000000000Z" );
    }
    SUBCASE("invalid")
    {
      KeyExtractor const ke{ Path::parse(".foo.bar"), format };
      CHECK( ke.extract(j) ==  "<< invalid key >>" );
    }
  }

  SUBCASE("UNIX_ms")
  {
    auto format = KeyExtractor::Format::UNIX_ms;
    SUBCASE("valid")
    {
      KeyExtractor const ke{ Path::parse(".unix_ms"), format };
      CHECK( ke.extract(j) ==  "2025-07-02T20:06:13.123000000Z" );
    }
    SUBCASE("invalid")
    {
      KeyExtractor const ke{ Path::parse(".foo.bar"), format };
      CHECK( ke.extract(j) ==  "<< invalid key >>" );
    }
  }

  SUBCASE("UNIX_us")
  {
    auto format = KeyExtractor::Format::UNIX_us;
    SUBCASE("valid")
    {
      KeyExtractor const ke{ Path::parse(".unix_us"), format };
      CHECK( ke.extract(j) ==  "2025-07-02T20:06:13.123456000Z" );
    }
    SUBCASE("invalid")
    {
      KeyExtractor const ke{ Path::parse(".foo.bar"), format };
      CHECK( ke.extract(j) ==  "<< invalid key >>" );
    }
  }

  SUBCASE("UNIX_ns")
  {
    auto format = KeyExtractor::Format::UNIX_ns;
    SUBCASE("valid")
    {
      KeyExtractor const ke{ Path::parse(".unix_ns"), format };
      CHECK( ke.extract(j) ==  "2025-07-02T20:06:13.123456789Z" );
    }
    SUBCASE("invalid")
    {
      KeyExtractor const ke{ Path::parse(".foo.bar"), format };
      CHECK( ke.extract(j) ==  "<< invalid key >>" );
    }
  }
}

}
}
