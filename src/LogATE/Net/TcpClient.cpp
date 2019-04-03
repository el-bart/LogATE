#include "LogATE/Net/TcpClient.hpp"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/SocketStream.h"

namespace LogATE::Net
{

TcpClient::TcpClient(std::string const& host, Port port)
{
  ss_.connect( Poco::Net::SocketAddress(host, port.value_) );
}

void TcpClient::write(nlohmann::json const& json)
{
  Poco::Net::SocketStream strm{ss_};
  strm << json;
}

}
