#include "CursATE/Screen/detail/id2key.hpp"
#include <doctest/doctest.h>

using CursATE::Screen::detail::id2key;
using CursATE::Screen::detail::key2id;

namespace
{
TEST_SUITE("CursATE::Screen::detail::id2key")
{


TEST_CASE("conversion back and forth")
{
  const auto key = LogATE::Log::Key{ "foo/bar", LogATE::SequenceNumber{42} };
  CHECK( key == id2key( key2id(key) ) );
}

}
}
