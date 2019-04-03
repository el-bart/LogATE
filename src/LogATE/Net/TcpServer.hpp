#pragma once
#include "LogATE/Net/Server.hpp"
//#include <Poco/Net/SocketAddress.h>
//#include <Poco/Net/Socket.h>
//#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/ServerSocket.h>
//#include <Poco/Net/SocketStream.h>
//#include <Poco/StreamCopier.h>
#include <Poco/Timespan.h>
//#include <Poco/FileStream.h>
#include <chrono>
#include <atomic>

namespace LogATE::Net
{

class TcpServer: public Server
{
public:
  struct Port { uint16_t value_; };

  explicit TcpServer(Port port);
  TcpServer(Port port, std::chrono::milliseconds pollTimeout, size_t queueSize);

  But::Optional<Log> readNextLog() override;
  void interrupt() override;

private:
//  struct ClientThread
  const Poco::Timespan pollTimeout_;
  std::atomic<bool> quit_{false};
//  std::vector<But::Treading::JoiningThread> threads_;
  Poco::Net::ServerSocket ss_;
};

/*
using namespace Poco;
using namespace Poco::Net;

int main()
{
        ServerSocket srvs(SocketAddress("localhost", 9999));
        Timespan span(250000);
        while (true)
        {
                if (srvs.poll(span, Socket::SELECT_READ))
                {
                        StreamSocket strs = srvs.acceptConnection();
                        SocketStream ostr(strs);
                        std::string file("test.in");
                        Poco::FileInputStream istr(file, std::ios::binary);
                        StreamCopier::copyStream(istr, ostr);
                }
        }
        return 0;
}
*/

}
