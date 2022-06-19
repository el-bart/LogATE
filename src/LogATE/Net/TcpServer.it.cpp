#include <doctest/doctest.h>
#include "LogATE/Net/TcpServer.hpp"
#include "LogATE/Net/TcpClient.hpp"
#include "LogATE/Net/TcpRawClient.hpp"
#include "LogATE/TestPrints.ut.hpp"
#include "But/Optional.hpp"

using LogATE::Log;
using LogATE::Tree::Path;
using LogATE::Net::Port;
using LogATE::Net::TcpServer;
using LogATE::Net::TcpClient;
using LogATE::Net::TcpRawClient;

namespace
{
TEST_SUITE("Net::TcpServer")
{

struct Fixture
{
  nlohmann::json str2json(std::string const& in) const { return nlohmann::json::parse(in); }

  const std::chrono::milliseconds serverTimeout_{1};
  const LogATE::Utils::WorkerThreadsShPtr workers_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>() };
  const Port port_{4242};
  const std::string host_{"127.0.0.1"};
  const nlohmann::json log1_{ str2json( R"({ "foo": "bar" })" ) };
  const nlohmann::json log2_{ str2json( R"({ "xxx": "yyy" })" ) };
  const TcpServer::JsonParsingMode parseMode_{TcpServer::JsonParsingMode::ParseToEndOfJson};
  const LogATE::Tree::Path keyPath_{ LogATE::Tree::Path::parse(".But::PreciseDT") };
};


TEST_CASE_FIXTURE(Fixture, "client fails to connect to a closed socket")
{
  CHECK_THROWS_WITH( (TcpClient{"127.6.6.6", Port{6666}}), "Connection refused" );
}


TEST_CASE_FIXTURE(Fixture, "server can handle one client")
{
  TcpServer s{workers_, port_, keyPath_, parseMode_, serverTimeout_};
  TcpClient c{host_, port_};
  CHECK( s.errors() == 0 );

  c.write(log1_);
  const auto logs = s.readNextLogs();
  CHECK( s.errors() == 0 );
  REQUIRE( logs.size() == 1u );
  CHECK( logs[0].json() == log1_ );
}


TEST_CASE_FIXTURE(Fixture, "server can handle one client and multiple messages")
{
  TcpServer s{workers_, port_, keyPath_, parseMode_, serverTimeout_};
  TcpClient c{host_, port_};

  for(auto const& json: {log1_, log2_})
  {
    c.write(json);
    const auto logs = s.readNextLogs();
    CHECK( s.errors() == 0 );
    REQUIRE( logs.size() == 1u );
    CHECK( logs[0].json() == json );
  }
}


TEST_CASE_FIXTURE(Fixture, "server does not disconnect on parse errors")
{
  TcpServer s{workers_, port_, keyPath_, parseMode_, serverTimeout_};
  TcpRawClient rc{host_, port_};

  CHECK( s.errors() == 0 );
  rc.write("}}}\n");
  rc.write( log1_.dump() );

  const auto logs = s.readNextLogs();
  CHECK( s.errors() == 3 );
  REQUIRE( logs.size() == 1u );
  CHECK( logs[0].json() == log1_ );
}


TEST_CASE_FIXTURE(Fixture, "server can handle json spread through multiple lines")
{
  TcpServer s{workers_, port_, keyPath_, parseMode_, serverTimeout_};
  TcpRawClient rc{host_, port_};

  const auto jsonStr = std::string{R"( {
                                         "one": 1,
                                         "two": []
                                       })"};
  rc.write(jsonStr);

  const auto logs = s.readNextLogs();
  CHECK( s.errors() == 0 );
  REQUIRE( logs.size() == 1u );
  CHECK( logs[0].json() == nlohmann::json::parse(jsonStr) );
}


TEST_CASE_FIXTURE(Fixture, "server can handle jsons attached to next other")
{
  TcpServer s{workers_, port_, keyPath_, parseMode_, serverTimeout_};
  TcpRawClient rc{host_, port_};

  const auto jsonStr = std::string{R"({"foo":"bar"})"};
  const auto count = 2;
  for(auto i=0; i<count; ++i)
    rc.write(jsonStr);

  for(auto i=0; i<count;)
  {
    const auto logs = s.readNextLogs();
    i += logs.size();
    CHECK( s.errors() == 0 );
    REQUIRE( not logs.empty() );
    for(auto& log: logs)
      CHECK( log.json() == nlohmann::json::parse(jsonStr) );
  }
}


TEST_CASE_FIXTURE(Fixture, "server gets back to accepting connections once current client disconnects")
{
  TcpServer s{workers_, port_, keyPath_, parseMode_, serverTimeout_};

  for(auto i=0; i<2; ++i)
  {
    TcpClient c{host_, port_};
    c.write(log1_);
    const auto logs = s.readNextLogs();
    CHECK( s.errors() == 0 );   // disconnection is not an error
    REQUIRE( logs.size() == 1u );
    CHECK( logs[0].json() == log1_ );
  }
}


TEST_CASE_FIXTURE(Fixture, "server can be stopped even when client is still connected")
{
  But::Optional<TcpRawClient> rc;
  TcpServer s{workers_, port_, keyPath_, parseMode_, serverTimeout_};
  rc.emplace(host_, port_);
}


TEST_CASE_FIXTURE(Fixture, "server can be stopped even when client is in a middle of json transfer")
{
  But::Optional<TcpRawClient> rc;
  TcpServer s{workers_, port_, keyPath_, parseMode_, serverTimeout_};
  rc.emplace(host_, port_);
  rc->write(R"({ "foo":)");
  std::this_thread::yield();
  s.interrupt();
}


TEST_CASE_FIXTURE(Fixture, "server recovers from an error in one of the JSONs")
{
  TcpServer s{workers_, port_, keyPath_, parseMode_, serverTimeout_};
  TcpRawClient c{host_, port_};

  c.write( log1_.dump() );
  c.write( "{{}}" );
  c.write( log2_.dump() );
  std::vector<LogATE::AnnotatedLog> out;

  do
  {
    const auto logs = s.readNextLogs();
    REQUIRE( not logs.empty() );
    for(auto& log: logs)
      out.push_back(log);
  }
  while( out.size() < 2u );

  CHECK( out[0].json() == log1_ );
  CHECK( out[1].json() == log2_ );
  CHECK( s.errors() == 1 );
}


TEST_CASE_FIXTURE(Fixture, "non-unique path is not allowed for keys")
{
  SUBCASE("empty path")
  {
    CHECK_THROWS_AS( (TcpServer{workers_, port_, Path{}, parseMode_, serverTimeout_}), TcpServer::InvalidKeyPath );
    CHECK_THROWS_AS( (TcpServer{workers_, port_, Path::parse("."), parseMode_, serverTimeout_}), TcpServer::InvalidKeyPath );
  }
  SUBCASE("non-asbolute path")
  {
    const auto path = Path::parse("non.absolute");
    CHECK_THROWS_AS( (TcpServer{workers_, port_, path, parseMode_, serverTimeout_}), TcpServer::InvalidKeyPath );
  }
  SUBCASE("non-unique path")
  {
    const auto path = Path::parse(".has.wildcards[].in.it");
    CHECK_THROWS_AS( (TcpServer{workers_, port_, path, parseMode_, serverTimeout_}), TcpServer::InvalidKeyPath );
  }
  SUBCASE("empty path makes no sense for a key")
  {
    const auto path = Path{};
    CHECK_THROWS_AS( (TcpServer{workers_, port_, path, parseMode_, serverTimeout_}), TcpServer::InvalidKeyPath );
  }
}


TEST_CASE_FIXTURE(Fixture, "non-wildcard arrays are allowed")
{
  CHECK_NOTHROW( (TcpServer{workers_, port_, Path::parse(".foo[42].bar[13].narf"), parseMode_, serverTimeout_}) );
}


// TODO: test hard break on a new line mode

}
}
