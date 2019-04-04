#include "LogATE/Net/TcpRawClient.hpp"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/SocketStream.h"

namespace LogATE::Net
{

TcpRawClient::TcpRawClient(std::string const& host, Port port)
{
  ss_.connect( Poco::Net::SocketAddress(host, port.value_) );
}

void TcpRawClient::write(std::string const& str)
{
  Poco::Net::SocketStream strm{ss_};
  strm << str;
}

}
