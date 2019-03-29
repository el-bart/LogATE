#include <doctest/doctest.h>
#include "LogATE/Tree/Logs.hpp"
#include "LogATE/TestPrints.ut.hpp"

using LogATE::Tree::Logs;
using SN = LogATE::SequenceNumber;

namespace
{

TEST_SUITE("Tree::Logs")
{

struct Fixture
{
  auto makeLog(unsigned num) const
  {
    auto log = LogATE::json2log(R"({ "foo": "bar" })");
    log.sn_.value_ = num;
    return log;
  }

  auto allSns() const
  {
    std::vector<SN> out;
    auto locked = logs_.withLock();
    out.reserve( locked->size() );
    for(auto it=locked->begin(); it!=locked->end(); ++it)
      out.push_back( it->sn_ );
    return out;
  }

  auto makeSns(std::initializer_list<unsigned> lst) const
  {
    std::vector<SN> out;
    out.reserve( lst.size() );
    for(auto sn: lst)
      out.push_back( SN{sn} );
    return out;
  }

  auto logs2sns(std::vector<LogATE::Log> const& in)
  {
    std::vector<SN> out;
    out.reserve(in.size());
    for(auto log: in)
      out.push_back(log.sn_);
    return out;
  }

  Logs logs_;
};

TEST_CASE_FIXTURE(Fixture, "empty set upon creation")
{
  CHECK( logs_.withLock()->size() == 0u );
  CHECK( logs_.withLock()->empty() );
}

TEST_CASE_FIXTURE(Fixture, "adding one element makes range usage")
{
  const auto tmp = makeLog(42);
  logs_.withLock()->insert(tmp);
  CHECK( not logs_.withLock()->empty() );
  CHECK( logs_.withLock()->size() == 1u );
  CHECK( logs_.withLock()->last().sn_ == tmp.sn_ );
  CHECK( logs_.withLock()->first().sn_ == tmp.sn_ );
}

TEST_CASE_FIXTURE(Fixture, "elements added in order are ordered correctly")
{
  for(auto i=0; i<10; ++i)
  {
    const auto tmp = makeLog(i);
    logs_.withLock()->insert(tmp);
    CHECK( not logs_.withLock()->empty() );
    CHECK( logs_.withLock()->size() == i+1 );
    CHECK( logs_.withLock()->last().sn_ == tmp.sn_ );
  }
}

TEST_CASE_FIXTURE(Fixture, "elements added in different order are auto-ordered correctly")
{
  const auto tmp1 = makeLog(41);
  const auto tmp2 = makeLog(43);
  logs_.withLock()->insert(tmp2);
  logs_.withLock()->insert(tmp1);
  CHECK( not logs_.withLock()->empty() );
  CHECK( logs_.withLock()->size() == 2u );
  CHECK( logs_.withLock()->last().sn_ == tmp2.sn_ );
  CHECK( logs_.withLock()->first().sn_ == tmp1.sn_ );
}

TEST_CASE_FIXTURE(Fixture, "prunning elements until given value")
{
  for(auto sn: {2,3,5,7,8,9})
    logs_.withLock()->insert( makeLog(sn) );
  CHECK( logs_.withLock()->pruneUpTo(SN{8}) == 4 );
  CHECK( allSns() == makeSns({8,9}) );
}

TEST_CASE_FIXTURE(Fixture, "prunning elements until non existing low value will not touch collection")
{
  for(auto sn: {2,3,5,7,8,9})
    logs_.withLock()->insert( makeLog(sn) );
  CHECK( logs_.withLock()->pruneUpTo(SN{1}) == 0 );
  CHECK( allSns() == makeSns({2,3,5,7,8,9}) );
}

TEST_CASE_FIXTURE(Fixture, "prunning elements until non existing high value will clean collection")
{
  for(auto sn: {2,3,5,7,8,9})
    logs_.withLock()->insert( makeLog(sn) );
  CHECK( logs_.withLock()->pruneUpTo(SN{42}) == 6 );
  CHECK( logs_.withLock()->empty() );
}

TEST_CASE_FIXTURE(Fixture, "getting given range")
{
  for(auto sn: {2,3,5,7,8,9})
    logs_.withLock()->insert( makeLog(sn) );
  CHECK( logs2sns( logs_.withLock()->range(SN{3}, SN{8})  ) == makeSns({3,5,7}) );
  CHECK( logs2sns( logs_.withLock()->range(SN{1}, SN{8})  ) == makeSns({2,3,5,7}) );
  CHECK( logs2sns( logs_.withLock()->range(SN{3}, SN{42}) ) == makeSns({3,5,7,8,9}) );
  CHECK( logs2sns( logs_.withLock()->range(SN{3}, SN{6})  ) == makeSns({3,5}) );
}

TEST_CASE_FIXTURE(Fixture, "getting count starting with a given position")
{
  for(auto sn: {2,3,5,7,8,9})
    logs_.withLock()->insert( makeLog(sn) );
  CHECK( logs2sns( logs_.withLock()->from(SN{3}, 0)  ) == makeSns({}) );
  CHECK( logs2sns( logs_.withLock()->from(SN{2}, 1)  ) == makeSns({2}) );
  CHECK( logs2sns( logs_.withLock()->from(SN{9}, 1)  ) == makeSns({9}) );
  CHECK( logs2sns( logs_.withLock()->from(SN{1}, 1)  ) == makeSns({2}) );
  CHECK( logs2sns( logs_.withLock()->from(SN{6}, 1)  ) == makeSns({7}) );
  CHECK( logs2sns( logs_.withLock()->from(SN{4}, 2)  ) == makeSns({5,7}) );
  CHECK( logs2sns( logs_.withLock()->from(SN{9}, 1)  ) == makeSns({9}) );
  CHECK( logs2sns( logs_.withLock()->from(SN{3}, 99)  ) == makeSns({3,5,7,8,9}) );
  CHECK( logs2sns( logs_.withLock()->from(SN{13}, 5)  ) == makeSns({}) );
  CHECK( logs2sns( logs_.withLock()->from(SN{0}, 99)  ) == makeSns({2,3,5,7,8,9}) );
}

TEST_CASE_FIXTURE(Fixture, "getting count ending with a given position")
{
  for(auto sn: {2,3,5,7,8,9})
    logs_.withLock()->insert( makeLog(sn) );
  CHECK( logs2sns( logs_.withLock()->to(SN{3}, 0)  ) == makeSns({}) );
  CHECK( logs2sns( logs_.withLock()->to(SN{99}, 0)  ) == makeSns({}) );
  CHECK( logs2sns( logs_.withLock()->to(SN{2}, 1)  ) == makeSns({2}) );
  CHECK( logs2sns( logs_.withLock()->to(SN{9}, 1)  ) == makeSns({9}) );
  CHECK( logs2sns( logs_.withLock()->to(SN{0}, 1)  ) == makeSns({}) );
  CHECK( logs2sns( logs_.withLock()->to(SN{13}, 1)  ) == makeSns({9}) );
  CHECK( logs2sns( logs_.withLock()->to(SN{9}, 2)  ) == makeSns({8,9}) );
  CHECK( logs2sns( logs_.withLock()->to(SN{9}, 6)  ) == makeSns({2,3,5,7,8,9}) );
  CHECK( logs2sns( logs_.withLock()->to(SN{9}, 666)  ) == makeSns({2,3,5,7,8,9}) );
  CHECK( logs2sns( logs_.withLock()->to(SN{5}, 666)  ) == makeSns({2,3,5}) );
  CHECK( logs2sns( logs_.withLock()->to(SN{5}, 2)  ) == makeSns({3,5}) );
}

}

}
