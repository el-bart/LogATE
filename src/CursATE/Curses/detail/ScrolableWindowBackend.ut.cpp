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
  ScreenSize ss_{Rows{3}, Columns{10}};
};

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
    CHECK( displayData().lines_ == dsSubset(0,1) );
    CHECK( displayData().currentSelection_.value_ == 42 );
    ds_->addNewest("foo2");
    CHECK( displayData().lines_ == dsSubset(0,2) );
    CHECK( displayData().currentSelection_.value_ == 42 );
    ds_->addNewest("foo3");
    CHECK( displayData().lines_ == dsSubset(0,3) );
    CHECK( displayData().currentSelection_.value_ == 42 );
  }
  SUBCASE("full-screen at once")
  {
    for(auto i=0; i<3; ++i)
      ds_->addNewest("foo" + std::to_string(i));
    CHECK( displayData().lines_ == dsSubset(0,3) );
    CHECK( displayData().currentSelection_.value_ == 42 );
  }
  SUBCASE("more than a single screen")
  {
    for(auto i=0; i<10; ++i)
      ds_->addNewest("foo" + std::to_string(i));
    CHECK( displayData().lines_ == dsSubset(0,3) );
    CHECK( displayData().currentSelection_.value_ == 42 );
  }
}

TEST_CASE_FIXTURE(Fixture, "iterating over bigger set of elements")
{
  for(auto i=0; i<30; ++i)
    ds_->addNewest("foo" + std::to_string(i));

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
  SUBCASE("moving down pass the end element does not move cursor")
  {
    /*
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
    */
  }
}


// TODO: tests for all possible moves

}
}
