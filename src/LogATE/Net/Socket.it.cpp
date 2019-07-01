#include <doctest/doctest.h>
#include <thread>
#include <But/Optional.hpp>
#include <But/Threading/JoiningThread.hpp>
#include "Socket.hpp"
#include "SocketDescriptorPair.hpp"

using Thread = But::Threading::JoiningThread<std::thread>;
using LogATE::Net::Exception;
using LogATE::Net::Socket;
using LogATE::Net::SocketDescriptorPair;

namespace
{
TEST_SUITE("LogATE::Net::Socket")
{

struct Fixture
{
  Fixture() { buf_.resize(10); }

  auto generate(size_t size) const
  {
    std::string out;
    out.reserve(size);
    const char tab[] = "abcdefghijklmnopqrstuwvxyz0123456789!@#$%^&*()_+ABCDEFGHIJKLMNOPQRSTUWVXYZ";
    constexpr auto tabSize = sizeof(tab) - 1;
    for (size_t i = 0; i < size; ++i)
      out.push_back(tab[i % tabSize]);
    return out;
  }

  SocketDescriptorPair sp_;
  std::string buf_;
};


TEST_CASE_FIXTURE(Fixture, "closed socket throws")
{
  CHECK_THROWS_AS( Socket{ But::System::Descriptor{} }, Exception );
}


TEST_CASE_FIXTURE(Fixture, "writing and reading")
{
  Socket s1{std::move(sp_.first)};
  Socket s2{std::move(sp_.second)};
  CHECK( s1.write("test!") == 5 );
  buf_.resize(5);
  {
    const auto ret = s2.read(buf_);
    CHECK( ret.first == Socket::Reason::Ok );
    CHECK( "test!" == ret.second );
  }
}


TEST_CASE_FIXTURE(Fixture, "interrupting socket affects only the first I/O operation")
{
  std::string data{"foo-bar"};
  Socket s1{std::move(sp_.first)};
  Socket s2{std::move(sp_.second)};

  for(auto i=0; i<5; ++i)
  {
    s1.interrupt();
    s2.interrupt();
  }

  CHECK( s1.write("ignored") == 0u );
  buf_.resize(7);
  {
    const auto ret = s2.read(buf_);
    CHECK( ret.first == Socket::Reason::Interrupted );
    CHECK( ret.second.size() == 0u );
  }

  CHECK(data.size() == s1.write(data));
  buf_.resize( data.size() );
  {
    const auto ret = s2.read(buf_);
    CHECK(ret.first == Socket::Reason::Ok);
    CHECK(data == ret.second);
  }
}


TEST_CASE_FIXTURE(Fixture, "interrputing reading")
{
  Socket writer{std::move(sp_.second)};
  Socket s{std::move(sp_.first)};
  Thread th{ [&]{
    for(auto i=0; i<100; ++i)
    {
      writer.write("write sth here...");
      std::this_thread::yield();
    }
    s.interrupt();
  } };
  buf_.resize(256*1024);
  {
    const auto ret = s.read(buf_);
    CHECK( ret.first == Socket::Reason::Interrupted );
    CHECK( 0u < ret.second.size() );
  }
}


TEST_CASE_FIXTURE(Fixture, "interrupting writing")
{
  Socket in{std::move(sp_.first)};
  Socket out{std::move(sp_.second)};
  Thread th{ [&]{
    buf_.resize(1);
    CHECK( out.read(buf_).second.size() == 1u );    // wait for sth to be written - i.e. write() blocks
    in.interrupt();
  } };
  const auto big = generate(256 * 1024);
  CHECK( in.write(big) < big.size() );
}


TEST_CASE_FIXTURE(Fixture, "async read and write")
{
  Socket in{std::move(sp_.first)};
  Socket out{std::move(sp_.second)};
  CHECK( in.write("123") == 3u );
  Thread th{ [&]{ CHECK( in.write("4567") == 4u ); } };
  buf_.resize(7);
  {
    const auto ret = out.read(buf_);
    CHECK( ret.first == Socket::Reason::Ok );
    CHECK( ret.second == "1234567" );
  }
}


TEST_CASE_FIXTURE(Fixture, "writing multi part data works fine")
{
  // NOTE: these numbers must be kept big - otherwise there is almost no chance for write() to get blocked
  constexpr auto chunkSize = 100u * 1024u;
  constexpr auto chunks = 10u;
  const auto data = generate(chunks * chunkSize);
  Socket in{std::move(sp_.first)};
  Socket out{std::move(sp_.second)};
  Thread th{ [&]{
    buf_.resize(chunkSize);
    for(auto i=0u; i<chunks; ++i)
    {
      const auto ret = out.read(buf_);
      CHECK( ret.first == Socket::Reason::Ok );
      CHECK( data.substr(i * chunkSize, chunkSize) == ret.second );
    }
  } };
  CHECK( data.size() == in.write(data) );
}


TEST_CASE_FIXTURE(Fixture, "reading multi part works fine")
{
  constexpr auto chunkSize = 200u;
  constexpr auto chunks = 10u;
  const auto data = generate(chunks * chunkSize);
  Socket in{std::move(sp_.first)};
  Socket out{std::move(sp_.second)};
  Thread th{ [&]{
    for(auto i=0u; i<chunks; ++i)
      CHECK( in.write(data.substr(i * chunkSize, chunkSize)) == chunkSize );
  } };
  buf_.resize( data.size() );
  {
    const auto ret = out.read(buf_);
    CHECK( ret.first == Socket::Reason::Ok );
    CHECK( data == ret.second );
  }
}


TEST_CASE_FIXTURE(Fixture, "reading from closed socket should return partial read")
{
  But::Optional<Socket> in{Socket{std::move(sp_.first)}};
  Thread th{ [&] { CHECK( in->write("x") == 1u ); in.reset(); } };
  Socket out{std::move(sp_.second)};
  buf_.resize(1024);
  {
    const auto ret = out.read(buf_);
    CHECK( ret.first == Socket::Reason::ClosedByRemoteEnd );
    CHECK( ret.second.size() == 1u );
  }
}


TEST_CASE_FIXTURE(Fixture, "readSome() blocks until any data is available")
{
  Socket s1{std::move(sp_.first)};
  Socket s2{std::move(sp_.second)};
  CHECK( s1.write("test!") == 5 );
  buf_.resize(1000);
  {
    const auto ret = s2.readSome(buf_);
    CHECK( ret.first == Socket::Reason::Ok );
    CHECK( "test!" == ret.second );
  }
}


TEST_CASE_FIXTURE(Fixture, "readSome() blocks until any data is available")
{
  Socket in{std::move(sp_.first)};
  Socket out{std::move(sp_.second)};
  Thread th{ [&] { CHECK( in.write("x") == 1u ); } };
  {
    const auto ret = out.readSome(buf_);
    CHECK( ret.first == Socket::Reason::Ok );
    CHECK( ret.second == "x" );
  }
}


TEST_CASE_FIXTURE(Fixture, "readSome() returns empty view on timeout")
{
  Socket s1{std::move(sp_.first)};
  Socket s2{std::move(sp_.second)};
  buf_.resize(1000);
  const auto timeout = std::chrono::milliseconds{1};
  const auto start = std::chrono::steady_clock::now();
  const auto ret = s2.readSome(buf_, timeout);
  const auto stop = std::chrono::steady_clock::now();
  CHECK( ret.first == Socket::Reason::Timeout );
  CHECK( ret.second.empty() );
  CHECK(stop-start >= timeout);
}


TEST_CASE_FIXTURE(Fixture, "reading to empty buffer returns no data as a reason")
{
  Socket s1{std::move(sp_.first)};
  Socket s2{std::move(sp_.second)};
  buf_.resize(0);
  SUBCASE("read()")
  {
    const auto ret = s1.read(buf_);
    CHECK( ret.first == Socket::Reason::NoData );
    CHECK( ret.second.empty() );
  }
  SUBCASE("readSome()")
  {
    const auto ret = s1.readSome(buf_);
    CHECK( ret.first == Socket::Reason::NoData );
    CHECK( ret.second.empty() );
  }
  SUBCASE("readSome(timeout)")
  {
    const auto ret = s1.readSome(buf_, std::chrono::seconds{13});
    CHECK( ret.first == Socket::Reason::NoData );
    CHECK( ret.second.empty() );
  }
}

}
}
