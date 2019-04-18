#include <doctest/doctest.h>
#include "LogATE/Tree/FilterFactory.hpp"
#include "LogATE/TestPrints.ut.hpp"

using LogATE::Tree::FilterFactory;
using LogATE::Tree::Node;
using Opts = LogATE::Tree::FilterFactory::Options;

namespace
{
TEST_SUITE("Tree::FilterFactory")
{

struct Fixture
{
  const Node::Name name_{"foo-bar"};
  FilterFactory ff_;
};


TEST_CASE_FIXTURE(Fixture, "constructing AcceptAll filter")
{
  const auto type = FilterFactory::Type{"AcceptAll"};

  SUBCASE("valid")
  {
    CHECK( ff_.build( type, name_, Opts{} )->type().value_ == type.name_ );
  }

  SUBCASE("unknown argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{{"foo", "bar"}} ), FilterFactory::UnknownOption );
  }
}


TEST_CASE_FIXTURE(Fixture, "constructing Explode filter")
{
  const auto type = FilterFactory::Type{"Explode"};

  SUBCASE("valid")
  {
    CHECK( ff_.build( type, name_, Opts{{"Path", ".foo.bar"}} )->type().value_ == type.name_ );
  }

  SUBCASE("missing argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{} ), FilterFactory::MissingOption );
  }

  SUBCASE("unknown argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{{"Path", ".foo.bar"}, {"foo", "bar"}} ), FilterFactory::UnknownOption );
  }

  SUBCASE("invalid argument")
  {
    CHECK_THROWS( ff_.build( type, name_, Opts{{"Path", ".invalid..path"}} ) );
  }
}


TEST_CASE_FIXTURE(Fixture, "constructing Grep filter")
{
  const auto type = FilterFactory::Type{"Grep"};

  SUBCASE("valid")
  {
    CHECK( ff_.build( type, name_, Opts{
                                         {"Path", ".foo.bar"},
                                         {"regex", "a.*"},
                                         {"Compare", "Key"},
                                         {"Case", "Sensitive"},
                                         {"Search", "Regular"}
                                       } )->type().value_ == type.name_ );
    CHECK( ff_.build( type, name_, Opts{
                                         {"Path", ".foo.bar"},
                                         {"regex", "a.*"},
                                         {"Compare", "Value"},
                                         {"Case", "Sensitive"},
                                         {"Search", "Regular"}
                                       } )->type().value_ == type.name_ );
    CHECK( ff_.build( type, name_, Opts{
                                         {"Path", ".foo.bar"},
                                         {"regex", "a.*"},
                                         {"Compare", "Key"},
                                         {"Case", "Insensitive"},
                                         {"Search", "Regular"}
                                       } )->type().value_ == type.name_ );
    CHECK( ff_.build( type, name_, Opts{
                                         {"Path", ".foo.bar"},
                                         {"regex", "a.*"},
                                         {"Compare", "Key"},
                                         {"Case", "Sensitive"},
                                         {"Search", "Inverse"}
                                       } )->type().value_ == type.name_ );
  }

  SUBCASE("missing argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"regex", "a.*"},
                                                   {"Compare", "Key"},
                                                   {"Case", "Sensitive"},
                                                   {"Search", "Regular"}
                                                 } ), FilterFactory::MissingOption );
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"Compare", "Key"},
                                                   {"Case", "Sensitive"},
                                                   {"Search", "Regular"}
                                                 } ), FilterFactory::MissingOption );
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"regex", "a.*"},
                                                   {"Case", "Sensitive"},
                                                   {"Search", "Regular"}
                                                 } ), FilterFactory::MissingOption );
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"regex", "a.*"},
                                                   {"Compare", "Key"},
                                                   {"Search", "Regular"}
                                                 } ), FilterFactory::MissingOption );
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"regex", "a.*"},
                                                   {"Compare", "Key"},
                                                   {"Case", "Sensitive"}
                                                 } ), FilterFactory::MissingOption );
  }

  SUBCASE("unknown argument")
  {
    CHECK_THROWS_AS( ff_.build( type, name_, Opts{
                                                   {"Path", ".foo.bar"},
                                                   {"regex", "a.*"},
                                                   {"Compare", "Key"},
                                                   {"Case", "Sensitive"},
                                                   {"Search", "Regular"},
                                                   {"foo", "bar"}
                                                 } ), FilterFactory::UnknownOption );
  }

  SUBCASE("invalid argument")
  {
    CHECK_THROWS( ff_.build( type, name_, Opts{
                                                {"Path", ".foo.bar"},
                                                {"regex", "** invalid regex **"},
                                                {"Compare", "Key"},
                                                {"Case", "Sensitive"},
                                                {"Search", "Regular"}
                                              } ) );
    CHECK_THROWS( ff_.build( type, name_, Opts{
                                                {"Path", ".invalid..path"},
                                                {"regex", ".*"},
                                                {"Compare", "Key"},
                                                {"Case", "Sensitive"},
                                                {"Search", "Regular"}
                                               } ) );
  }
}

}
}
