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


TEST_CASE_FIXTURE(Fixture, "getting key in raw SourceFormat")
{
  nlohmann::json j;
  j["iso"]     = "2025-07-02T20:06:13.123456789Z";
  j["unix"]    = 1751486773;
  j["unix_ms"] = 1751486773123;
  j["unix_us"] = 1751486773123456;
  j["unix_ns"] = 1751486773123456789;
  j["str-unix"]    = "1751486773";
  j["str-unix_ms"] = "1751486773123";
  j["str-unix_us"] = "1751486773123456";
  j["str-unix_ns"] = "1751486773123456789";
  j["foo"]["bar"] = "sth";
  j["float"] = 3.14;
  j["str-float"] = "3.14";

  SUBCASE("getting path back")
  {
    KeyExtractor const ke{ Path::parse(".foo.bar"), KeyExtractor::SourceFormat::Raw };
    CHECK( ke.path().str() == ".foo.bar" );
  }

  SUBCASE("raw SourceFormat")
  {
    auto const sourceFormat = KeyExtractor::SourceFormat::Raw;
    SUBCASE("existing path (string)")
    {
      KeyExtractor const ke{ Path::parse(".foo.bar"), sourceFormat };
      CHECK( ke(j) ==  "sth" );
    }
    SUBCASE("existing path (numeric)")
    {
      KeyExtractor const ke{ Path::parse(".unix"), sourceFormat };
      CHECK( ke(j) ==  "1751486773" );
    }
    SUBCASE("non-existing path")
    {
      KeyExtractor const ke{ Path::parse(".not.there"), sourceFormat };
      CHECK( ke(j) == "<< key not found >>" );
    }
  }

  SUBCASE("ISO8601_ns")
  {
    auto const sourceFormat = KeyExtractor::SourceFormat::ISO8601_ns;
    SUBCASE("valid")
    {
      KeyExtractor const ke{ Path::parse(".iso"), sourceFormat };
      CHECK( ke(j) ==  "2025-07-02T20:06:13.123456789Z" );
    }
    SUBCASE("invalid")
    {
      KeyExtractor const ke{ Path::parse(".foo.bar"), sourceFormat };
      CHECK( ke(j) ==  "<< invalid key >>" );
    }
  }

  SUBCASE("UNIX")
  {
    auto const sourceFormat = KeyExtractor::SourceFormat::UNIX;
    SUBCASE("valid")
    {
      KeyExtractor const ke{ Path::parse(".unix"), sourceFormat };
      CHECK( ke(j) ==  "2025-07-02T20:06:13.000000000Z" );
    }
    SUBCASE("valid, but string")
    {
      KeyExtractor const ke{ Path::parse(".str-unix"), sourceFormat };
      CHECK( ke(j) ==  "2025-07-02T20:06:13.000000000Z" );
    }
    SUBCASE("invalid string")
    {
      KeyExtractor const ke{ Path::parse(".foo.bar"), sourceFormat };
      CHECK( ke(j) ==  "<< invalid key >>" );
    }
    SUBCASE("invalid float")
    {
      KeyExtractor const ke{ Path::parse(".float"), sourceFormat };
      CHECK( ke(j) ==  "<< invalid key >>" );
    }
    SUBCASE("invalid float as string")
    {
      KeyExtractor const ke{ Path::parse(".str-float"), sourceFormat };
      CHECK( ke(j) ==  "<< invalid key >>" );
    }
  }

  SUBCASE("UNIX_ms")
  {
    auto const sourceFormat = KeyExtractor::SourceFormat::UNIX_ms;
    SUBCASE("valid")
    {
      KeyExtractor const ke{ Path::parse(".unix_ms"), sourceFormat };
      CHECK( ke(j) ==  "2025-07-02T20:06:13.123000000Z" );
    }
    SUBCASE("valid, but string")
    {
      KeyExtractor const ke{ Path::parse(".str-unix_ms"), sourceFormat };
      CHECK( ke(j) ==  "2025-07-02T20:06:13.123000000Z" );
    }
    SUBCASE("invalid string")
    {
      KeyExtractor const ke{ Path::parse(".foo.bar"), sourceFormat };
      CHECK( ke(j) ==  "<< invalid key >>" );
    }
    SUBCASE("invalid float")
    {
      KeyExtractor const ke{ Path::parse(".float"), sourceFormat };
      CHECK( ke(j) ==  "<< invalid key >>" );
    }
    SUBCASE("invalid float as string")
    {
      KeyExtractor const ke{ Path::parse(".str-float"), sourceFormat };
      CHECK( ke(j) ==  "<< invalid key >>" );
    }
  }

  SUBCASE("UNIX_us")
  {
    auto const sourceFormat = KeyExtractor::SourceFormat::UNIX_us;
    SUBCASE("valid")
    {
      KeyExtractor const ke{ Path::parse(".unix_us"), sourceFormat };
      CHECK( ke(j) ==  "2025-07-02T20:06:13.123456000Z" );
    }
    SUBCASE("valid, but string")
    {
      KeyExtractor const ke{ Path::parse(".str-unix_us"), sourceFormat };
      CHECK( ke(j) ==  "2025-07-02T20:06:13.123456000Z" );
    }
    SUBCASE("invalid string")
    {
      KeyExtractor const ke{ Path::parse(".foo.bar"), sourceFormat };
      CHECK( ke(j) ==  "<< invalid key >>" );
    }
    SUBCASE("invalid float")
    {
      KeyExtractor const ke{ Path::parse(".float"), sourceFormat };
      CHECK( ke(j) ==  "<< invalid key >>" );
    }
    SUBCASE("invalid float as string")
    {
      KeyExtractor const ke{ Path::parse(".str-float"), sourceFormat };
      CHECK( ke(j) ==  "<< invalid key >>" );
    }
  }

  SUBCASE("UNIX_ns")
  {
    auto const sourceFormat = KeyExtractor::SourceFormat::UNIX_ns;
    SUBCASE("valid")
    {
      KeyExtractor const ke{ Path::parse(".unix_ns"), sourceFormat };
      CHECK( ke(j) ==  "2025-07-02T20:06:13.123456789Z" );
    }
    SUBCASE("valid, but string")
    {
      KeyExtractor const ke{ Path::parse(".str-unix_ns"), sourceFormat };
      CHECK( ke(j) ==  "2025-07-02T20:06:13.123456789Z" );
    }
    SUBCASE("invalid string")
    {
      KeyExtractor const ke{ Path::parse(".foo.bar"), sourceFormat };
      CHECK( ke(j) ==  "<< invalid key >>" );
    }
    SUBCASE("invalid float")
    {
      KeyExtractor const ke{ Path::parse(".float"), sourceFormat };
      CHECK( ke(j) ==  "<< invalid key >>" );
    }
    SUBCASE("invalid float as string")
    {
      KeyExtractor const ke{ Path::parse(".str-float"), sourceFormat };
      CHECK( ke(j) ==  "<< invalid key >>" );
    }
  }

  SUBCASE("non-unique path is not allowed for keys")
  {
    auto const sourceFormat = KeyExtractor::SourceFormat::UNIX_ns;
    SUBCASE("empty path")
    {
      CHECK_THROWS_AS( (KeyExtractor{Path{}, sourceFormat}), KeyExtractor::InvalidKeyPath );
      CHECK_THROWS_AS( (KeyExtractor{Path::parse("."), sourceFormat}), KeyExtractor::InvalidKeyPath );
    }
    SUBCASE("non-asbolute path")
    {
      CHECK_THROWS_AS( (KeyExtractor{Path::parse("non.absolute"), sourceFormat}), KeyExtractor::InvalidKeyPath );
    }
    SUBCASE("non-unique path")
    {
      CHECK_THROWS_AS( (KeyExtractor{Path::parse(".has.wildcards[].in.it"), sourceFormat}), KeyExtractor::InvalidKeyPath );
    }
  }

  SUBCASE("non-wildcard arrays are allowed")
  {
    auto const sourceFormat = KeyExtractor::SourceFormat::UNIX_ns;
    CHECK_NOTHROW( KeyExtractor{ Path::parse(".foo[42].bar[13].narf"), sourceFormat } );
  }
}

}
}
