#pragma once
#include "LogATE/Net/Port.hpp"
#include <nlohmann/json.hpp>
#include <Poco/Net/StreamSocket.h>
#include <string>

namespace LogATE::Net
{

class TcpClient final
{
public:
  TcpClient(std::string const& host, Port port);

  void write(nlohmann::json const& json);

private:
  Poco::Net::StreamSocket ss_;
};

}
