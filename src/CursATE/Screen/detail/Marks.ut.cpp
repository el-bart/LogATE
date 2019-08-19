#include <doctest/doctest.h>
#include "CursATE/Screen/detail/Marks.hpp"
#include "CursATE/Curses/detail/StringDataSource.ut.hpp"
#include "LogATE/TestHelpers.ut.hpp"

using CursATE::Screen::detail::Marks;
using CursATE::Curses::ScrolableWindow;
using CursATE::Curses::detail::StringDataSource;
using LogATE::makeKey;

namespace
{
TEST_SUITE("CursATE::Screen::detail::Marks")
{

struct Fixture
{
  auto makeWindow() const
  {
#if 0
    using CursATE::Curses::ScreenPosition;
    using CursATE::Curses::Column;
    using CursATE::Curses::Row;
    const auto sp = ScreenPosition{ Row{1}, Column{1} };
    using CursATE::Curses::ScreenSize;
    using CursATE::Curses::Columns;
    using CursATE::Curses::Rows;
    const auto ss = ScreenSize{ Rows{10}, Columns{20} };
    using CursATE::Curses::Window;
    const auto sds_ = But::makeSharedNN<StringDataSource>();
#else
    // cannot return actuall window, as there is no ncurses initialized in these tests.
    // this will work just fine, as long the window is not de-references inside the SUT.
    static auto ptrCounter = 0x12345;
    const auto dummyPtr = reinterpret_cast<ScrolableWindow*>(++ptrCounter);
    const auto noopDeleter = [](auto*) { };
    return std::shared_ptr<ScrolableWindow>{dummyPtr, noopDeleter};
#endif
  }

  Fixture()
  {
    BUT_ASSERT( makeWindow().get() != makeWindow().get() && "new instance is not returned!" );
  }

  Marks m_;
};


TEST_CASE_FIXTURE(Fixture, "empy by default")
{
  CHECK( m_.size() == 0u );
  CHECK_FALSE( m_.find('x') );
}


TEST_CASE_FIXTURE(Fixture, "caching and getting")
{
  const auto key1 = makeKey();
  const auto win1 = makeWindow();
  m_.insert( 'a', Marks::Entry{key1, win1} );
  {
    const auto e = m_.find('a');
    REQUIRE(static_cast<bool>(e) == true);
    CHECK( e->key_ == key1 );
    const auto win = e->window_.lock();
    CHECK( win.get() == win1.get() );
  }

  {
    const auto e = m_.find('b');
    REQUIRE(static_cast<bool>(e) == false);
  }

  const auto key2 = makeKey();
  const auto win2 = makeWindow();
  m_.insert( 'b', Marks::Entry{key2, win2} );
  {
    const auto e = m_.find('a');
    REQUIRE(static_cast<bool>(e) == true);
    CHECK( e->key_ == key1 );
    const auto win = e->window_.lock();
    CHECK( win.get() == win1.get() );
  }
  {
    const auto e = m_.find('b');
    REQUIRE( static_cast<bool>(e) == true);
    CHECK( e->key_ == key2 );
    const auto win = e->window_.lock();
    CHECK( win.get() == win2.get() );
  }
}


TEST_CASE_FIXTURE(Fixture, "insertiuon for existing key overrides")
{
  const auto key1 = makeKey();
  const auto win1 = makeWindow();
  m_.insert( 'a', Marks::Entry{key1, win1} );

  const auto key2 = makeKey();
  const auto win2 = makeWindow();
  m_.insert( 'a', Marks::Entry{key2, win2} );

  const auto e = m_.find('a');
  REQUIRE(static_cast<bool>(e) == true);
  CHECK( e->key_ == key2 );
  const auto win = e->window_.lock();
  CHECK( win.get() == win2.get() );
}


TEST_CASE_FIXTURE(Fixture, "getting invalidated entry")
{
  auto key1 = makeKey();
  auto win1 = makeWindow();
  m_.insert( 'a', Marks::Entry{key1, win1} );
  win1.reset();

  const auto e = m_.find('a');
  REQUIRE(static_cast<bool>(e) == true);
  CHECK( e->key_ == key1 );
  const auto win = e->window_.lock();
  CHECK( win.get() == nullptr );
}


TEST_CASE_FIXTURE(Fixture, "prunning removes invalidated entries")
{
  auto key1 = makeKey();
  auto win1 = makeWindow();
  m_.insert( 'a', Marks::Entry{key1, win1} );

  auto key2 = makeKey();
  auto win2 = makeWindow();
  m_.insert( 'x', Marks::Entry{key2, win2} );

  CHECK( m_.size() == 2u );
  win1.reset();
  CHECK( m_.size() == 2u );
  m_.prune();
  CHECK( m_.size() == 1u );

  {
    const auto e = m_.find('a');
    CHECK(static_cast<bool>(e) == false);
  }
  {
    const auto e = m_.find('x');
    REQUIRE(static_cast<bool>(e) == true);
    CHECK( e->key_ == key2 );
    const auto win = e->window_.lock();
    CHECK( win.get() == win2.get() );
  }
}

}
}
