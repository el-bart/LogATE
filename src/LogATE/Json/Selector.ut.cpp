#include <doctest/doctest.h>
#include "LogATE/Json/Selector.hpp"


namespace
{
TEST_SUITE("Json::Selector")
{

struct Fixture
{
  LogATE::Json::Selector s_;
};


TEST_CASE_FIXTURE(Fixture, "initial state is empty")
{
  CHECK( s_.str() == "" );
  CHECK( s_.jsonComplete() == false );

  SUBCASE("reseting empty object does nto change its state")
  {
    s_.reset();
    CHECK( s_.str() == "" );
    CHECK( s_.jsonComplete() == false );
  }
}

}
}
