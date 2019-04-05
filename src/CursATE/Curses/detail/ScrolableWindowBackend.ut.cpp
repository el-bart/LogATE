#include <doctest/doctest.h>
#include "CursATE/Curses/detail/ScrolableWindowBackend.hpp"
#include "CursATE/Curses/detail/StringDataSource.ut.hpp"
#include "CursATE/Curses/TestPrints.ut.hpp"

using CursATE::Curses::detail::ScrolableWindowBackend;
using CursATE::Curses::detail::StringDataSource;
using CursATE::Curses::DataSource;
using CursATE::Curses::ScreenSize;
using CursATE::Curses::Columns;
using CursATE::Curses::Rows;

using DisplayData = ScrolableWindowBackend::DisplayData;
using Id = DataSource::Id;

namespace
{
TEST_SUITE("CursATE::Curses::detail::ScrolableWindowBackend")
{

struct Fixture
{
  template<typename ...Args>
  auto lines(Args ...args) const
  {
    return std::vector<std::pair<DataSource::Id, std::string>>{args...};
  }
  auto line(unsigned id, std::string const& str) const
  {
    return std::make_pair(Id{id}, str);
  }

  But::NotNullShared<StringDataSource> ds_{ But::makeSharedNN<StringDataSource>() };
  ScrolableWindowBackend swb_{ds_};
  ScreenSize ss_{Rows{3}, Columns{10}};
};

TEST_CASE_FIXTURE(Fixture, "no elements yield empty result")
{
  CHECK( swb_.displayData(ss_).lines_.empty() );
  CHECK( swb_.displayData(ss_).currentSelection_ == Id{0} );
}

TEST_CASE_FIXTURE(Fixture, "adding elements and displaying right away")
{
  ds_->addNewest("str#1");
  REQUIRE( swb_.displayData(ss_).lines_ == lines( line(42, "str#1") ) );
  REQUIRE( swb_.displayData(ss_).currentSelection_ == Id{42} );

  ds_->addNewest("str#2");
  REQUIRE( swb_.displayData(ss_).lines_ == lines( line(42, "str#1"), line(43, "str#2") ) );
  REQUIRE( swb_.displayData(ss_).currentSelection_ == Id{42} );

  ds_->addNewest("str#3");
  REQUIRE( swb_.displayData(ss_).lines_ == lines( line(42, "str#1"), line(43, "str#2"), line(45, "str#3") ) );
  REQUIRE( swb_.displayData(ss_).currentSelection_ == Id{42} );

  ds_->addNewest("str#4");
  REQUIRE( swb_.displayData(ss_).lines_ == lines( line(42, "str#1"), line(43, "str#2"), line(45, "str#3") ) );
  REQUIRE( swb_.displayData(ss_).currentSelection_ == Id{42} );
}

TEST_CASE_FIXTURE(Fixture, "adding multiple elements and then displaying")
{
  ds_->addNewest("str#1");
  ds_->addNewest("str#2");
  ds_->addNewest("str#3");
  REQUIRE( swb_.displayData(ss_).lines_ == lines( line(42, "str#1"), line(43, "str#2"), line(45, "str#3") ) );
  REQUIRE( swb_.displayData(ss_).currentSelection_ == Id{42} );
}

TEST_CASE_FIXTURE(Fixture, "adding more elements than lines elements and then displaying")
{
  ds_->addNewest("str#1");
  ds_->addNewest("str#2");
  ds_->addNewest("str#3");
  ds_->addNewest("str#4");
  REQUIRE( swb_.displayData(ss_).lines_ == lines( line(42, "str#1"), line(43, "str#2"), line(45, "str#3") ) );
  REQUIRE( swb_.displayData(ss_).currentSelection_ == Id{42} );
}

TEST_CASE_FIXTURE(Fixture, "iterating over empty elements list does not explode")
{
  swb_.scrollUp();
  REQUIRE( swb_.displayData(ss_).lines_ == lines() );
  REQUIRE( swb_.displayData(ss_).currentSelection_ == Id{} );

  swb_.scrollDown();
  REQUIRE( swb_.displayData(ss_).lines_ == lines() );
  REQUIRE( swb_.displayData(ss_).currentSelection_ == Id{} );

  swb_.scrollLeft();
  REQUIRE( swb_.displayData(ss_).lines_ == lines() );
  REQUIRE( swb_.displayData(ss_).currentSelection_ == Id{} );

  swb_.scrollRight();
  REQUIRE( swb_.displayData(ss_).lines_ == lines() );
  REQUIRE( swb_.displayData(ss_).currentSelection_ == Id{} );
}

TEST_CASE_FIXTURE(Fixture, "iterating over elements back and forth")
{
  ds_->addNewest("str#1");
  ds_->addNewest("str#2");
  ds_->addNewest("str#3");
  ds_->addNewest("str#4");
  REQUIRE( swb_.displayData(ss_).lines_ == lines( line(42, "str#1"), line(43, "str#2"), line(45, "str#3") ) );
  REQUIRE( swb_.displayData(ss_).currentSelection_ == Id{42} );
  //db_->
}


// TODO: tests for all possible moves

}
}
