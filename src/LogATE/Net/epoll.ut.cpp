#include <doctest/doctest.h>
#include <thread>
#include <But/Threading/JoiningThread.hpp>
#include "LogATE/Net/SocketDescriptorPair.hpp"
#include "LogATE/Net/epoll.hpp"

using Thread = But::Threading::JoiningThread<std::thread>;
using LogATE::Net::epoll;
using LogATE::Net::ReadyFor;
using LogATE::Net::SocketDescriptorPair;

namespace
{
TEST_SUITE("LogATE::Net::epoll")
{

struct Fixture
{
  auto write(But::System::Descriptor const& fd, std::string const& data) const
  {
    return ::write(fd.get(), data.data(), data.size());
  }
};

TEST_CASE_FIXTURE(Fixture, "no arguments return nullptr")
{
  CHECK( epoll( {} ) == nullptr );
}

TEST_CASE_FIXTURE(Fixture, "waiting for change on read descriptor")
{
  SocketDescriptorPair sdp[2];
  CHECK( write(sdp[1].first, "x") == 1 );
  const auto ret = epoll( { {&sdp[0].second, ReadyFor::Read}, {&sdp[1].second, ReadyFor::Read} } );
  CHECK( &sdp[1].second == ret );
}

TEST_CASE_FIXTURE(Fixture, "waiting for async change on read descriptors")
{
  SocketDescriptorPair sdp[2];
  Thread th{ [&]{ CHECK( write(sdp[0].first, "x") == 1 ); } };
  const auto ret = epoll( { {&sdp[0].second, ReadyFor::Read}, {&sdp[1].second, ReadyFor::Read} } );
  CHECK( &sdp[0].second == ret );
}

TEST_CASE_FIXTURE(Fixture, "waiting for async change on write descriptors")
{
  SocketDescriptorPair sdp;
  const auto ret = epoll( { {&sdp.second, ReadyFor::Write} } );
  CHECK( &sdp.second == ret );
}

TEST_CASE_FIXTURE(Fixture, "waiting for async change on read write descriptors")
{
  SocketDescriptorPair sdp;
  Thread th{ [&]{ CHECK( write(sdp.first, "x") == 1 ); } };
  const auto ret = epoll( { {&sdp.second, ReadyFor::ReadWrite} } );
  CHECK( &sdp.second == ret );
}


TEST_CASE_FIXTURE(Fixture, "timeout returns nullptr")
{
  const auto timeout = std::chrono::milliseconds{3};
  SocketDescriptorPair sdp[2];
  const auto start = std::chrono::steady_clock::now();
  const auto ret = epoll( { {&sdp[0].second, ReadyFor::Read}, {&sdp[1].second, ReadyFor::Read} }, timeout );
  const auto stop = std::chrono::steady_clock::now();
  CHECK( ret == nullptr );
  CHECK( stop-start >= timeout );

}

}
}
