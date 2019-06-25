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

  SocketDescriptorPair sp;
};


TEST_CASE_FIXTURE(Fixture, "closed socket throws")
{
  CHECK_THROWS_AS( Socket{ But::System::Descriptor{} }, Exception );
}


TEST_CASE_FIXTURE(Fixture, "writing and reading")
{
  Socket s1{std::move(sp.first)};
  Socket s2{std::move(sp.second)};
  CHECK( s1.write("test!") == 5 );
  CHECK( "test!" == s2.read(5) );
}


TEST_CASE_FIXTURE(Fixture, "interrupting socket affects only the first I/O operation")
{
  std::string data{"foo-bar"};
  Socket s1{std::move(sp.first)};
  Socket s2{std::move(sp.second)};

  for(auto i=0; i<5; ++i)
  {
    s1.interrupt();
    s2.interrupt();
  }

  CHECK( s1.write("ignored") == 0u );
  CHECK( s2.read(7).size() == 0u );

  CHECK(data.size() == s1.write(data));
  CHECK(data == s2.read(data.size()));
}


TEST_CASE_FIXTURE(Fixture, "interrputing reading")
{
  Socket writer{std::move(sp.second)};
  Socket s{std::move(sp.first)};
  Thread th{ [&]{
    for(auto i=0; i<100; ++i)
    {
      writer.write("write sth here...");
      std::this_thread::yield();
    }
    s.interrupt();
  } };
  CHECK( 0u < s.read(256 * 1024).size() );
}


TEST_CASE_FIXTURE(Fixture, "interrupting writing")
{
  Socket in{std::move(sp.first)};
  Socket out{std::move(sp.second)};
  Thread th{ [&]{
    CHECK( out.read(1).size() == 1u );  // wait for sth to be written - i.e. write() blocks
    in.interrupt();
  } };
  const auto big = generate(256 * 1024);
  CHECK( in.write(big) < big.size() );
}


TEST_CASE_FIXTURE(Fixture, "async read and write")
{
  Socket in{std::move(sp.first)};
  Socket out{std::move(sp.second)};
  CHECK( in.write("123") == 3u );
  Thread th{ [&]{ CHECK( in.write("4567") == 4u ); } };
  CHECK( out.read(7) == "1234567" );
}


TEST_CASE_FIXTURE(Fixture, "writing multi part data works fine")
{
  // NOTE: these numbers must be kept big - otherwise there is almost no chance for write() to get blocked
  constexpr auto chunkSize = 100u * 1024u;
  constexpr auto chunks = 10u;
  const auto data = generate(chunks * chunkSize);
  Socket in{std::move(sp.first)};
  Socket out{std::move(sp.second)};
  Thread th{ [&]{
    for(auto i=0u; i<chunks; ++i)
      CHECK( data.substr(i * chunkSize, chunkSize) == out.read(chunkSize) );
  } };
  CHECK( data.size() == in.write(data) );
}


TEST_CASE_FIXTURE(Fixture, "reading multi part works fine")
{
  constexpr auto chunkSize = 200u;
  constexpr auto chunks = 10u;
  const auto data = generate(chunks * chunkSize);
  Socket in{std::move(sp.first)};
  Socket out{std::move(sp.second)};
  Thread th{ [&]{
    for(auto i=0u; i<chunks; ++i)
      CHECK( in.write(data.substr(i * chunkSize, chunkSize)) == chunkSize );
  } };
  CHECK( data == out.read(data.size()) );
}


TEST_CASE_FIXTURE(Fixture, "reading from closed socket should return partial read")
{
  But::Optional<Socket> in{Socket{std::move(sp.first)}};
  Thread th{ [&] { CHECK( in->write("x") == 1u ); in.reset(); } };
  Socket out{std::move(sp.second)};
  CHECK( out.read(1024).size() == 1u );
}

}
}
