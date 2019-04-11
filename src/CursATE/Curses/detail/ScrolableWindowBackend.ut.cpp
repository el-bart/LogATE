#include <doctest/doctest.h>
#include "CursATE/Curses/detail/ScrolableWindowBackend.hpp"
#include "CursATE/Curses/detail/StringDataSource.ut.hpp"

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
  Fixture()
  {
    update();
  }

  void update()
  {
    swb_.resize(ss_);
    swb_.update();
  }

  auto displayData()
  {
    update();
    return swb_.displayData();
  }

  But::NotNullShared<StringDataSource> ds_{ But::makeSharedNN<StringDataSource>() };
  ScrolableWindowBackend swb_{ds_};
  ScreenSize ss_{Rows{3}, Columns{10}};
};

TEST_CASE_FIXTURE(Fixture, "no elements yield empty result")
{
  for(auto i=0; i<3; ++i)
    CHECK( displayData().lines_.empty() );
}

TEST_CASE_FIXTURE(Fixture, "adding elements until screen size")
{
  SUBCASE("one at a time")
  {
    ds_->addNewest("foo");
    CHECK( displayData().lines_.size() == 1 );
    CHECK( displayData().currentSelection_.value_ == 42 );
    ds_->addNewest("foo");
    CHECK( displayData().lines_.size() == 2 );
    CHECK( displayData().currentSelection_.value_ == 42 );
    ds_->addNewest("foo");
    CHECK( displayData().lines_.size() == 3 );
    CHECK( displayData().currentSelection_.value_ == 42 );
  }
  SUBCASE("full-screen at once")
  {
    ds_->addNewest("foo");
    ds_->addNewest("foo");
    ds_->addNewest("foo");
    CHECK( displayData().lines_.size() == 3 );
    CHECK( displayData().currentSelection_.value_ == 42 );
  }
}


// TODO: tests for all possible moves

}
}
