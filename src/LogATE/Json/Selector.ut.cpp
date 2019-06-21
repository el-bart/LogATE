#include <doctest/doctest.h>
#include "LogATE/Json/Selector.hpp"

using LogATE::Json::Selector;

namespace
{
TEST_SUITE("Json::Selector")
{

struct Fixture
{
  void update(std::string const& in)
  {
    for(auto c: in)
      s_.update(c);
  }

  Selector s_;
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


TEST_CASE_FIXTURE(Fixture, "cannot start with an unknown character")
{
  CHECK_THROWS_AS( s_.update('x'), Selector::UnexpectedCharacter );
  CHECK_THROWS_AS( s_.update('q'), Selector::UnexpectedCharacter );
  CHECK_THROWS_AS( s_.update('!'), Selector::UnexpectedCharacter );
}


TEST_CASE_FIXTURE(Fixture, "whitespaces at the begining are consummed")
{
  s_.update(' ');  CHECK( s_.str() == "" );
  s_.update('\t'); CHECK( s_.str() == "" );
  s_.update('\r'); CHECK( s_.str() == "" );
  s_.update('\n'); CHECK( s_.str() == "" );
}


TEST_CASE_FIXTURE(Fixture, "whitespaces at the begining are consummed")
{
  s_.update(' ');  CHECK( s_.str() == "" );
  s_.update('\t'); CHECK( s_.str() == "" );
  s_.update('\r'); CHECK( s_.str() == "" );
  s_.update('\n'); CHECK( s_.str() == "" );
}


TEST_CASE_FIXTURE(Fixture, "parsing string")
{
  s_.update('"'); CHECK( s_.str() == R"(")" );
  update("foo bar");
  s_.update('"'); CHECK( s_.str() == R"("foo bar")" );
  CHECK( s_.jsonComplete() );
}


TEST_CASE_FIXTURE(Fixture, "parsing string with escape characters")
{
  update(R"("foo \"\r\n bar")");
  CHECK( s_.str() == R"("foo \"\r\n bar")" );
  CHECK( s_.jsonComplete() );
}

// TODO: array
// TODO: object
// TODO: bools
// TODO: bools
// TODO: number

// TODO: nested elements

}
}
