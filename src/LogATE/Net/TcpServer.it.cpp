#include <doctest/doctest.h>
#include "LogATE/Net/TcpServer.hpp"
#include "LogATE/Net/TcpClient.hpp"
#include "LogATE/TestPrints.ut.hpp"

using LogATE::Log;
using LogATE::Net::Port;
using LogATE::Net::TcpServer;
using LogATE::Net::TcpClient;

namespace
{
TEST_SUITE("Net::TcpServer")
{

struct Fixture
{
  nlohmann::json str2json(std::string const& in) const { return nlohmann::json::parse(in); }

  const Port port_{4242};
  const std::string host_{"127.0.0.1"};
  const nlohmann::json log1_{ str2json( R"({ "foo": "bar" })" ) };
  const nlohmann::json log2_{ str2json( R"({ "xxx": "yyy" })" ) };
};

TEST_CASE_FIXTURE(Fixture, "client fails to connect to a closed socket")
{
  CHECK_THROWS_WITH( (TcpClient{"127.6.6.6", Port{6666}}), "Connection refused" );
}

TEST_CASE_FIXTURE(Fixture, "server can handle one client")
{
  TcpServer s{port_};
  TcpClient c{host_, port_};
  c.write(log1_);
  const auto log = s.readNextLog();
  REQUIRE(log);
  CHECK( *log->log_ == log1_ );
}

}
}
