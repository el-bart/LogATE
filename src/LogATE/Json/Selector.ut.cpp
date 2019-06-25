#include <doctest/doctest.h>
#include "LogATE/Json/Selector.hpp"

using LogATE::Json::Selector;

//#define PARSER_DETECTS_ELABORATE_ERRORS

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


#ifdef PARSER_DETECTS_ELABORATE_ERRORS
TEST_CASE_FIXTURE(Fixture, "cannot start with an unknown character")
{
  CHECK_THROWS_AS( s_.update('x'), Selector::UnexpectedCharacter );
  CHECK_THROWS_AS( s_.update('q'), Selector::UnexpectedCharacter );
  CHECK_THROWS_AS( s_.update('!'), Selector::UnexpectedCharacter );
}
#endif


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


TEST_CASE_FIXTURE(Fixture, "reset is clearing state")
{
  SUBCASE("in the middle of parsing")
  {
    update(R"("foo )");
    s_.reset();
    CHECK( s_.str() == "" );
    CHECK( not s_.jsonComplete() );
  }
  SUBCASE("after the parsing")
  {
    update(R"("foo bar")");
    s_.reset();
    CHECK( s_.str() == "" );
    CHECK( not s_.jsonComplete() );
  }
}


TEST_CASE_FIXTURE(Fixture, "parsing string with escape characters")
{
  update(R"("foo \"\r\n bar")");
  CHECK( s_.str() == R"("foo \"\r\n bar")" );
  CHECK( s_.jsonComplete() );
}


TEST_CASE_FIXTURE(Fixture, "parsing boolean true value")
{
  update(R"(true)");
  CHECK( s_.str() == R"(true)" );
  CHECK( s_.jsonComplete() );
}


#ifdef PARSER_DETECTS_ELABORATE_ERRORS
TEST_CASE_FIXTURE(Fixture, "parsing invalid boolean true value")
{
  update(R"(tr)");
  CHECK_THROWS_AS( s_.update('X'), Selector::InvalidBoolean );
}
#endif


TEST_CASE_FIXTURE(Fixture, "parsing boolean false value")
{
  update(R"(false)");
  CHECK( s_.str() == R"(false)" );
  CHECK( s_.jsonComplete() );
}


#ifdef PARSER_DETECTS_ELABORATE_ERRORS
TEST_CASE_FIXTURE(Fixture, "parsing invalid boolean false value")
{
  update(R"(fal)");
  CHECK_THROWS_AS( s_.update('X'), Selector::InvalidBoolean );
}
#endif


TEST_CASE_FIXTURE(Fixture, "parsing null")
{
  update(R"(null)");
  CHECK( s_.str() == R"(null)" );
  CHECK( s_.jsonComplete() );
}


#ifdef PARSER_DETECTS_ELABORATE_ERRORS
TEST_CASE_FIXTURE(Fixture, "parsing invalid null")
{
  update(R"(nu)");
  CHECK_THROWS_AS( s_.update('X'), Selector::InvalidNull );
  s_.update('l');
  CHECK_THROWS_AS( s_.update('X'), Selector::InvalidNull );
}
#endif


TEST_CASE_FIXTURE(Fixture, "end of stream")
{
  SUBCASE("empty stream is ok")
  {
    s_.eos();
    CHECK( not s_.jsonComplete() );
    CHECK( s_.str() == "" );
  }
  SUBCASE("ending a number")
  {
    update("42");
#ifdef PARSER_DETECTS_ELABORATE_ERRORS
    CHECK( not s_.jsonComplete() );
#endif
    s_.eos();
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == "42" );
  }
  SUBCASE("ending with non-empty and non-number state is an error")
  {
    update(R"("xx)");
    CHECK( not s_.jsonComplete() );
    CHECK_THROWS_AS( s_.eos(), Selector::UnexpectedEndOfStream );
  }
}


TEST_CASE_FIXTURE(Fixture, "parsing valid number")
{
  for(auto sign: {"", "-"})
    for(auto front: {"0", "123", "10"})
      for(auto dot: {"", "."})
        for(auto back: {"", "0", "01", "12345"})
          for(auto exp: {"e", "E"})
            for(auto expSign: {"", "-", "+"})
              for(auto expDig: {"0", "1", "6789"})
              {
                std::stringstream ss;
                ss << sign << front << dot << back << exp << expSign << expDig;
                const auto n = ss.str();
                update(n);
#ifdef PARSER_DETECTS_ELABORATE_ERRORS
                CHECK( not s_.jsonComplete() );
#endif
                s_.eos();
                CHECK( s_.jsonComplete() );
                CHECK( s_.str() == n );
                s_.reset();
              }
}


TEST_CASE_FIXTURE(Fixture, "parsing valid object")
{
  SUBCASE("empty object")
  {
    update("{}");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == "{}" );
  }
  SUBCASE("empty object with whitespaces")
  {
    update("\t\r\n {\r\t\n } \r\t\n");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == "{}" );
  }
  SUBCASE("object with simple key-value pair - string")
  {
    update(R"({  "foo"   :  "b a r"  })");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == R"({"foo":"b a r"})" );
  }
  SUBCASE("object with simple key-value pair - number")
  {
    update(R"({  "foo"   :  4.2  })");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == R"({"foo":4.2})" );
  }
  SUBCASE("object with simple key-value pair - bool/true")
  {
    update(R"({  "foo"   :  true  })");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == R"({"foo":true})" );
  }
  SUBCASE("object with simple key-value pair - bool/false")
  {
    update(R"({  "foo"   :  false })");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == R"({"foo":false})" );
  }
  SUBCASE("object with simple key-value pair - null")
  {
    update(R"({  "foo"   :  null })");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == R"({"foo":null})" );
  }
  SUBCASE("object with multiple key-value pairs (2 elements)")
  {
    update(R"({ "foo" : "b a a r"  , "answer": 42 })");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == R"({"foo":"b a a r","answer":42})" );
  }
  SUBCASE("object with multiple key-value pairs (3 elements)")
  {
    update(R"({ "xx":false,"foo" : "b a a r"  , "answer": 42 })");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == R"({"xx":false,"foo":"b a a r","answer":42})" );
  }
}


TEST_CASE_FIXTURE(Fixture, "parsing invalid object throws")
{
  SUBCASE("missing end bracket")
  {
    update("{");
    CHECK_THROWS_AS( s_.eos(), Selector::UnexpectedEndOfStream );
    CHECK( not s_.jsonComplete() );
    CHECK( s_.str() == "{" );
  }
#ifdef PARSER_DETECTS_ELABORATE_ERRORS
  SUBCASE("key must be a string")
  {
    update("{ ");
    CHECK_THROWS_AS( s_.update('4'), Selector::UnexpectedCharacter );
    CHECK( not s_.jsonComplete() );
    CHECK( s_.str() == "{" );
  }
  SUBCASE("collon is a must")
  {
    update(R"({ "foo" )");
    CHECK_THROWS_AS( s_.update('4'), Selector::UnexpectedCharacter );
    CHECK( not s_.jsonComplete() );
    CHECK( s_.str() == R"({"foo")" );
  }
  SUBCASE("object must be key-value")
  {
    update(R"({ "foo" )");
    CHECK_THROWS_AS( s_.update('}'), Selector::UnexpectedCharacter );
    CHECK( not s_.jsonComplete() );
    CHECK( s_.str() == R"({"foo")" );
  }
#endif
}


TEST_CASE_FIXTURE(Fixture, "parsing valid array")
{
  SUBCASE("empty array")
  {
    update("[]");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == "[]" );
  }
  SUBCASE("empty array with spaces")
  {
    update(" \t\r\n[ \t\r\n] \r\n\t");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == "[]" );
  }
  SUBCASE("one element array - string")
  {
    update(R"(["foo bar"])");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == R"(["foo bar"])" );
  }
  SUBCASE("one element array - number")
  {
    update(R"([42.666])");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == R"([42.666])" );
  }
  SUBCASE("one element array - bool/true")
  {
    update(R"([true])");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == R"([true])" );
  }
  SUBCASE("one element array - bool/false")
  {
    update(R"([false])");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == R"([false])" );
  }
  SUBCASE("multi element array")
  {
    update(R"([false,true,"narf"])");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == R"([false,true,"narf"])" );
  }
  SUBCASE("multi element array with spaces")
  {
    update(" [ false , true\r\t \n, \r\t\n \"narf\" ] ");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == R"([false,true,"narf"])" );
  }
}


TEST_CASE_FIXTURE(Fixture, "parsing invalid array throws")
{
  SUBCASE("missing array closing")
  {
    update(" [ ");
    CHECK_THROWS_AS( s_.eos(), Selector::UnexpectedEndOfStream );
    CHECK( not s_.jsonComplete() );
    CHECK( s_.str() == R"([)" );
  }
  SUBCASE("wrong element")
  {
    update(" [ false , ");
#ifdef PARSER_DETECTS_ELABORATE_ERRORS
    CHECK_THROWS_AS( s_.update('x'), Selector::UnexpectedCharacter );
#endif
    CHECK( not s_.jsonComplete() );
    CHECK( s_.str() == R"([false,)" );
  }
}


TEST_CASE_FIXTURE(Fixture, "parsing nested elements")
{
  SUBCASE("nested objects")
  {
    update(R"( { "narf" : { "foo" : "bar" } } )");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == R"({"narf":{"foo":"bar"}})" );
  }
  SUBCASE("array of objects")
  {
    update(R"( [ 42, { "raspberry": 3.14 }, { "true": false } ] )");
    CHECK( s_.jsonComplete() );
    CHECK( s_.str() == R"([42,{"raspberry":3.14},{"true":false}])" );
  }
}

}
}
