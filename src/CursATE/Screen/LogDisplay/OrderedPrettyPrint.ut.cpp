#include <doctest/doctest.h>
#include "CursATE/Screen/LogDisplay/OrderedPrettyPrint.hpp"

using CursATE::Screen::LogDisplay::OrderedPrettyPrint;

namespace
{
TEST_SUITE("CursATE::Screen::LogDisplay::OrderedPrettyPrint")
{


TEST_CASE("directly converting to pretty print")
{
  const OrderedPrettyPrint opp_;
  (void)opp_;
  FAIL("TODO...");
}


TEST_CASE("converting with priorities")
{
  const OrderedPrettyPrint opp_{{"foo", "bar"}};
  (void)opp_;
  FAIL("TODO...");
};

}
}
