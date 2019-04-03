#include "LogATE/Net/TcpClient.hpp"
#include "LogATE/Log.hpp"

int main()
{
  LogATE::Net::TcpClient c{"127.0.0.1", LogATE::Net::Port{6666}};
  c.write( nlohmann::json::parse(R"({ "foo": "bar" })") );
  c.write( nlohmann::json::parse(R"({ "answer": 42 })") );
}
