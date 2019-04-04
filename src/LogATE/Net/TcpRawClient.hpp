#pragma once
#include "LogATE/Net/Port.hpp"
#include <Poco/Net/StreamSocket.h>
#include <string>

namespace LogATE::Net
{

class TcpRawClient final
{
public:
  TcpRawClient(std::string const& host, Port port);

  void write(std::string const& str);

private:
  Poco::Net::StreamSocket ss_;
};

}
