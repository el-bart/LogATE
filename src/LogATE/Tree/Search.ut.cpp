#include <doctest/doctest.h>
#include "But/Threading/Event.hpp"
#include "LogATE/Tree/Search.hpp"
#include "LogATE/TestHelpers.ut.hpp"
#include "LogATE/TestPrints.ut.hpp"

using LogATE::Tree::Search;
using LogATE::Tree::Logs;
using LogATE::Log;


namespace
{
TEST_SUITE("Tree::Search")
{

struct Fixture
{
  auto populate(std::initializer_list<int> ns) const
  {
    std::vector<Log> out;
    out.reserve( ns.size() );
    auto counter = 0u;
    for(auto e: ns)
    {
      auto json = nlohmann::json::object();
      json["answer"] = e;
      auto log = LogATE::makeLog( counter++, json.dump() );
      out.push_back(log);
      logs_->withLock()->insert( std::move(log) );
    }
    return out;
  }

  auto waitFor(Search::Result& res) const
  {
    return res.value_.wait_for(timeout_) == std::future_status::ready;
  }

  LogATE::Utils::WorkerThreadsShPtr workers_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>(3) };
  But::NotNullShared<Logs> logs_{ But::makeSharedNN<Logs>() };
  Search s_{workers_, 5};
  const Search::Direction forward_{Search::Direction::Forward};
  const Search::Direction backward_{Search::Direction::Backward};
  const std::chrono::seconds timeout_{42};
};


struct LookForAnswer
{
  bool operator()(Log const& log) const
  {
    return log.json()["answer"].get<int>() == n_;
  }

  int n_{0};
};


TEST_CASE_FIXTURE(Fixture, "searching within one block")
{
  const auto data = populate({1,2,3,4});
  SUBCASE("forward")
  {
    auto result = s_.search( logs_, data[0].key(), forward_, LookForAnswer{3} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(opt);
    CHECK( *opt == data[2].key() );
  }
  SUBCASE("backward")
  {
    auto result = s_.search( logs_, data[3].key(), backward_, LookForAnswer{2} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(opt);
    CHECK( *opt == data[1].key() );
  }
}


TEST_CASE_FIXTURE(Fixture, "start element is included in the search")
{
  const auto data = populate({1,2,3,4,5});
  SUBCASE("forward")
  {
    auto result = s_.search( logs_, data[3].key(), forward_, LookForAnswer{4} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(opt);
    CHECK( *opt == data[3].key() );
  }
  SUBCASE("backward")
  {
    auto result = s_.search( logs_, data[3].key(), backward_, LookForAnswer{4} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(opt);
    CHECK( *opt == data[3].key() );
  }
}


TEST_CASE_FIXTURE(Fixture, "searching within multiple blocks")
{
  const auto data = populate({1,2,3,4,5, 1,2,3,4,5, 6,7,8,9,10});
  SUBCASE("forward")
  {
    auto result = s_.search( logs_, data[0].key(), forward_, LookForAnswer{4} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(opt);
    CHECK( *opt == data[3].key() );
  }
  SUBCASE("backward")
  {
    auto result = s_.search( logs_, data[14].key(), backward_, LookForAnswer{4} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(opt);
    CHECK( *opt == data[8].key() );
  }
}


struct BlockingQuery
{
  bool operator()(Log const&) const
  {
    waitForStart_->set();
    REQUIRE( unleash_->wait(timeout_) );
    return true;
  }

  std::chrono::seconds timeout_;
  But::NotNullShared<But::Threading::Event> waitForStart_;
  But::NotNullShared<But::Threading::Event> unleash_;
};

TEST_CASE_FIXTURE(Fixture, "canceling search")
{
  REQUIRE( workers_->threads() == 3 );
  const auto data = populate({1,2,3,4,5, 1,2,3,4,5, 6,7,8,9,10});
  auto waitForStart = But::makeSharedNN<But::Threading::Event>();
  auto unleash = But::makeSharedNN<But::Threading::Event>();
  const BlockingQuery query{timeout_, waitForStart, unleash};
  SUBCASE("forward")
  {
    auto result = s_.search( logs_, data[0].key(), forward_, query);
    REQUIRE( waitForStart->wait(timeout_) );
    *result.cancel_ = true;
    unleash->set();
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(not opt);
  }
  SUBCASE("backward")
  {
    auto result = s_.search( logs_, data[14].key(), backward_, query);
    REQUIRE( waitForStart->wait(timeout_) );
    *result.cancel_ = true;
    unleash->set();
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(not opt);
  }
}


TEST_CASE_FIXTURE(Fixture, "searcing with multiple matches returns first one")
{
  const auto data = populate({1,2,3,2,1});
  SUBCASE("forward")
  {
    auto result = s_.search( logs_, data[0].key(), forward_, LookForAnswer{2} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(opt);
    CHECK( *opt == data[1].key() );
  }
  SUBCASE("backward")
  {
    auto result = s_.search( logs_, data[4].key(), backward_, LookForAnswer{2} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(opt);
    CHECK( *opt == data[3].key() );
  }
}


struct ThrowOnceAndLookForAnswer
{
  explicit ThrowOnceAndLookForAnswer(int n): lfa_{n} { }

  bool operator()(Log const& log)
  {
    if(not alreadyThrown_)
    {
      alreadyThrown_ = true;
      throw std::runtime_error{"some expected error"};
    };
    return lfa_(log);
  }

  bool alreadyThrown_{false};
  LookForAnswer lfa_;
};

TEST_CASE_FIXTURE(Fixture, "exception in query does not break the search")
{
  const auto data = populate({1,2,3,4,5});
  SUBCASE("forward")
  {
    auto result = s_.search( logs_, data[0].key(), forward_, ThrowOnceAndLookForAnswer{3} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(opt);
    CHECK( *opt == data[2].key() );
  }
  SUBCASE("backward")
  {
    auto result = s_.search( logs_, data[4].key(), backward_, ThrowOnceAndLookForAnswer{3} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(opt);
    CHECK( *opt == data[2].key() );
  }
}


TEST_CASE_FIXTURE(Fixture, "non-existing elements are not found")
{
  const auto data = populate({1,2,3,4,5});
  SUBCASE("forward")
  {
    auto result = s_.search( logs_, data[2].key(), forward_, LookForAnswer{42} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(not opt);
  }
  SUBCASE("backward")
  {
    auto result = s_.search( logs_, data[2].key(), backward_, LookForAnswer{42} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(not opt);
  }
}


TEST_CASE_FIXTURE(Fixture, "elements not in range are not found")
{
  const auto data = populate({1,2,3,4,5});
  SUBCASE("forward")
  {
    auto result = s_.search( logs_, data[2].key(), forward_, LookForAnswer{2} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(not opt);
  }
  SUBCASE("backward")
  {
    auto result = s_.search( logs_, data[2].key(), backward_, LookForAnswer{4} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(not opt);
  }
}


TEST_CASE_FIXTURE(Fixture, "searching empty set does not return anything")
{
  SUBCASE("forward")
  {
    auto result = s_.search( logs_, LogATE::makeKey(1), forward_, LookForAnswer{2} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(not opt);
  }
  SUBCASE("backward")
  {
    auto result = s_.search( logs_, LogATE::makeKey(1), backward_, LookForAnswer{2} );
    REQUIRE( waitFor(result) );
    const auto opt = result.value_.get();
    REQUIRE(not opt);
  }
}


TEST_CASE_FIXTURE(Fixture, "required compares count is reasonable") // though not exact
{
  SUBCASE("empty set")
  {
    const auto data = populate({});
    auto result = s_.search( logs_, LogATE::makeKey(42), forward_, LookForAnswer{3} );
    CHECK( result.value_.wait_for(timeout_) == std::future_status::ready );
    CHECK( result.requiredCompares_->load() == data.size() );
    CHECK( result.comparesDone_->load() == 0u );
    CHECK( result.comparesDone_->load() <= *result.requiredCompares_ );
  }
  SUBCASE("less than one block")
  {
    const auto data = populate({1,2,3,4});
    auto result = s_.search( logs_, data[0].key(), forward_, LookForAnswer{3} );
    CHECK( result.value_.wait_for(timeout_) == std::future_status::ready );
    CHECK( result.requiredCompares_->load() == data.size() );
    CHECK( result.comparesDone_->load() > 0u );
    CHECK( result.comparesDone_->load() <= *result.requiredCompares_ );
  }
  SUBCASE("multiple blocks")
  {
    const auto data = populate({1,2,3,4,5, 6,7,8,9,10});
    auto result = s_.search( logs_, data[0].key(), forward_, LookForAnswer{3} );
    CHECK( result.value_.wait_for(timeout_) == std::future_status::ready );
    CHECK( result.requiredCompares_->load() >= data.size() );
    CHECK( result.comparesDone_->load() > 0u );
    CHECK( result.comparesDone_->load() <= *result.requiredCompares_ );
  }
}

}
}
