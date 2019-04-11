#include <doctest/doctest.h>
#include "CursATE/Curses/detail/ScrolableWindowBackend.hpp"
#include "CursATE/Curses/detail/StringDataSource.ut.hpp"
#include "CursATE/Curses/TestPrints.ut.hpp"
#include <iterator>

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
    swb_.resize(ss_);
  }

  auto displayData()
  {
    return swb_.displayData();
  }

  auto dsSubset(size_t from, size_t to) const
  {
    BUT_ASSERT( from <= to );
    BUT_ASSERT( to <= ds_->size() );
    std::vector<std::pair<DataSource::Id, std::string>> out;
    auto begin = ds_->data_.begin();
    std::advance(begin, from);
    auto end = ds_->data_.begin();
    std::advance(end, to);
    for(auto it = begin; it != end; ++it)
      out.push_back(*it);
    return out;
  }

  But::NotNullShared<StringDataSource> ds_{ But::makeSharedNN<StringDataSource>() };
  ScrolableWindowBackend swb_{ds_};
  const ScreenSize ss_{Rows{3}, Columns{10}};
};


TEST_CASE_FIXTURE(Fixture, "minimal window size is respected")
{
  SUBCASE("resize to 0xN")
  {
    auto tmp = ss_;
    tmp.rows_.value_ = 0;
    CHECK_THROWS_AS( swb_.resize(tmp), ScrolableWindowBackend::WindowTooSmall );
  }
  SUBCASE("resize to Nx0")
  {
    auto tmp = ss_;
    tmp.columns_.value_ = 0;
    CHECK_THROWS_AS( swb_.resize(tmp), ScrolableWindowBackend::WindowTooSmall );
  }
}


TEST_CASE_FIXTURE(Fixture, "default-sized window yields some results, regardless of number of entries in a DB")
{
  ScrolableWindowBackend swb{ds_};
  swb.update();
  CHECK( swb.displayData().lines_.size() == 0 );

  ds_->addNewest("foo");
  swb.update();
  CHECK( swb.displayData().lines_.size() == 1 );

  ds_->addNewest("bar");
  swb.update();
  CHECK( swb.displayData().lines_.size() == 1 );
}


TEST_CASE_FIXTURE(Fixture, "no elements yield empty result")
{
  for(auto i=0; i<3; ++i)
  {
    CHECK( displayData().lines_.empty() );
    CHECK( displayData().lines_ == dsSubset(0,0) ); // sanity check ;)
  }
}


TEST_CASE_FIXTURE(Fixture, "adding elements starting from a default selection")
{
  SUBCASE("one at a time")
  {
    ds_->addNewest("foo1");
    swb_.update();
    CHECK( displayData().lines_ == dsSubset(0,1) );
    CHECK( displayData().currentSelection_.value_ == 42 );
    ds_->addNewest("foo2");
    swb_.update();
    CHECK( displayData().lines_ == dsSubset(0,2) );
    CHECK( displayData().currentSelection_.value_ == 42 );
    ds_->addNewest("foo3");
    swb_.update();
    CHECK( displayData().lines_ == dsSubset(0,3) );
    CHECK( displayData().currentSelection_.value_ == 42 );
  }
  SUBCASE("full-screen at once")
  {
    for(auto i=0; i<3; ++i)
      ds_->addNewest("foo" + std::to_string(i));
    INFO("source data buffer: " << ds_->data_);
    swb_.update();
    CHECK( displayData().lines_ == dsSubset(0,3) );
    CHECK( displayData().currentSelection_.value_ == 42 );
  }
  SUBCASE("more than a single screen")
  {
    for(auto i=0; i<10; ++i)
      ds_->addNewest("foo" + std::to_string(i));
    INFO("source data buffer: " << ds_->data_);
    swb_.update();
    CHECK( displayData().lines_ == dsSubset(0,3) );
    CHECK( displayData().currentSelection_.value_ == 42 );
  }
}


TEST_CASE_FIXTURE(Fixture, "iterating over bigger set of elements")
{
  for(auto i=0; i<30; ++i)
    ds_->addNewest("foo" + std::to_string(i));
  INFO("source data buffer: " << ds_->data_);
  swb_.update();

  SUBCASE("moving up at the end of the top does not change anything")
  {
    const auto pre = displayData();
    swb_.selectUp();
    const auto post = displayData();
    CHECK( pre.lines_ == post.lines_ );
    CHECK( pre.currentSelection_ == post.currentSelection_ );
  }

  SUBCASE("moving down one element at a time")
  {
    CHECK( displayData().lines_ == dsSubset(0,3) );
    CHECK( displayData().currentSelection_.value_ == 42 );
    swb_.selectDown();
    CHECK( displayData().lines_ == dsSubset(0,3) );
    CHECK( displayData().currentSelection_.value_ == 43 );
    swb_.selectDown();
    CHECK( displayData().lines_ == dsSubset(0,3) );
    CHECK( displayData().currentSelection_.value_ == 45 );
    swb_.selectDown();
    CHECK( displayData().lines_ == dsSubset(1,4) );
    CHECK( displayData().currentSelection_.value_ == 48 );
    swb_.selectDown();
    CHECK( displayData().lines_ == dsSubset(2,5) );
    CHECK( displayData().currentSelection_.value_ == 52 );

    swb_.selectUp();
    CHECK( displayData().lines_ == dsSubset(2,5) );
    CHECK( displayData().currentSelection_.value_ == 48 );
    swb_.selectUp();
    CHECK( displayData().lines_ == dsSubset(2,5) );
    CHECK( displayData().currentSelection_.value_ == 45 );
    swb_.selectUp();
    CHECK( displayData().lines_ == dsSubset(1,4) );
    CHECK( displayData().currentSelection_.value_ == 43 );
    swb_.selectUp();
    CHECK( displayData().lines_ == dsSubset(0,3) );
    CHECK( displayData().currentSelection_.value_ == 42 );
    swb_.selectUp();
    CHECK( displayData().lines_ == dsSubset(0,3) );
    CHECK( displayData().currentSelection_.value_ == 42 );
  }

  SUBCASE("selecting page by page")
  {
    swb_.selectDown();
    CHECK( displayData().lines_ == dsSubset(0,3) );
    CHECK( displayData().currentSelection_.value_ == 43 );

    swb_.selectPageDown();
    CHECK( displayData().lines_ == dsSubset(2,5) );
    CHECK( displayData().currentSelection_.value_ == 52 );
    swb_.selectPageDown();
    CHECK( displayData().lines_ == dsSubset(5,8) );
    CHECK( displayData().currentSelection_.value_ == 70 );

    swb_.selectPageUp();
    CHECK( displayData().lines_ == dsSubset(4,7) );
    CHECK( displayData().currentSelection_.value_ == 52 );
    swb_.selectPageUp();
    CHECK( displayData().lines_ == dsSubset(1,4) );
    CHECK( displayData().currentSelection_.value_ == 43 );
  }

  SUBCASE("jumping to first/last elements")
  {
    swb_.selectLast();
    CHECK( displayData().lines_ == dsSubset(27,30) );
    CHECK( displayData().currentSelection_.value_ == 477 );

    swb_.selectFirst();
    CHECK( displayData().lines_ == dsSubset(0,3) );
    CHECK( displayData().currentSelection_.value_ == 42 );
  }
}


TEST_CASE_FIXTURE(Fixture, "iterating over empty data set")
{
  INFO("source data buffer: " << ds_->data_);

  for(auto i=0; i<2; ++i)
  {
    CHECK( displayData().lines_.empty() );
    swb_.selectUp();
    CHECK( displayData().lines_.empty() );
    swb_.selectDown();
    CHECK( displayData().lines_.empty() );
    swb_.selectPageDown();
    CHECK( displayData().lines_.empty() );
    swb_.selectPageDown();
    CHECK( displayData().lines_.empty() );
    swb_.selectLast();
    CHECK( displayData().lines_.empty() );
    swb_.selectFirst();
    CHECK( displayData().lines_.empty() );
  }
}


TEST_CASE_FIXTURE(Fixture, "resizing window on a fly")
{
  for(auto i=0; i<5; ++i)
    ds_->addNewest("foo" + std::to_string(i));
  INFO("source data buffer: " << ds_->data_);

  swb_.selectDown();
  REQUIRE( displayData().lines_ == dsSubset(0,3) );
  REQUIRE( displayData().currentSelection_.value_ == 43 );

  SUBCASE("increasing size")
  {
    auto tmp = ss_;
    tmp.rows_.value_ = 4;
    swb_.resize(tmp);
    CHECK( displayData().lines_ == dsSubset(0,4) );
    CHECK( displayData().currentSelection_.value_ == 43 );
  }

  SUBCASE("increasing size beyond window size")
  {
    auto tmp = ss_;
    tmp.rows_.value_ = 10;
    swb_.resize(tmp);
    CHECK( displayData().lines_ == dsSubset(0,5) );
    CHECK( displayData().currentSelection_.value_ == 43 );
  }

  SUBCASE("decreasing size")
  {
    auto tmp = ss_;
    tmp.rows_.value_ = 2;
    swb_.resize(tmp);
    CHECK( displayData().lines_ == dsSubset(0,2) );
    CHECK( displayData().currentSelection_.value_ == 43 );
  }

  SUBCASE("decreasing size such that current selection is out of scope")
  {
    auto tmp = ss_;
    tmp.rows_.value_ = 1;
    swb_.resize(tmp);
    CHECK( displayData().lines_ == dsSubset(1,2) );
    CHECK( displayData().currentSelection_.value_ == 43 );
  }
}


TEST_CASE_FIXTURE(Fixture, "resizing window on a fly, when data set is empty")
{
  REQUIRE( ds_->data_.empty() );
  auto tmp = ss_;
  tmp.rows_.value_ = 2;
  swb_.resize(tmp);
  CHECK( displayData().lines_.empty() );
}

// TODO: tests line scrolling by character
// TODO: tests line scrolling to begin/end

// TODO: test when selection is no longer in the input set (should reset to the latest log)

}
}
