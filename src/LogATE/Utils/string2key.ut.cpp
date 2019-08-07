#include "LogATE/Utils/string2key.hpp"
#include <doctest/doctest.h>

using LogATE::Utils::string2key;

namespace
{
TEST_SUITE("LogATE::Utils::string2key")
{


TEST_CASE("valid conversions")
{
  CHECK( LogATE::Log::Key{ "foo bar", LogATE::SequenceNumber{42} } == string2key("foo bar/00042") );
  CHECK( LogATE::Log::Key{ "foo/bar", LogATE::SequenceNumber{42} } == string2key("foo/bar/00042") );
  CHECK( LogATE::Log::Key{ "foo-bar", LogATE::SequenceNumber{42} } == string2key("foo-bar/42") );
  CHECK( LogATE::Log::Key{ "foo-bar", LogATE::SequenceNumber{42} } == string2key("foo-bar/0000000000042") );
}


TEST_CASE("invalid conversions")
{
  CHECK_THROWS_AS( string2key("foo-bar 042"), LogATE::Utils::StringDoesNotRepresentKey );
}

}
}
