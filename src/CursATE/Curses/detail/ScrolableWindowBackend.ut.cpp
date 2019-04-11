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
  But::NotNullShared<StringDataSource> ds_{ But::makeSharedNN<StringDataSource>() };
  ScrolableWindowBackend swb_{ds_};
  ScreenSize ss_{Rows{3}, Columns{10}};
};

TEST_CASE_FIXTURE(Fixture, "no elements yield empty result")
{
  CHECK( swb_.displayData(ss_).lines_.empty() );
}

TEST_CASE_FIXTURE(Fixture, "adding elements until screen size")
{
  CHECK( swb_.displayData(ss_).lines_.size() );
}


// TODO: tests for all possible moves

}
}
