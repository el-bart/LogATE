#include "LogATE/Net/TcpServer.hpp"
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/Socket.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>

namespace LogATE::Net
{

TcpServer::TcpServer(const Port port):
  TcpServer{port, std::chrono::seconds{2}}
{ }

TcpServer::TcpServer(const Port port, const std::chrono::milliseconds pollTimeout):
  pollTimeout_{ pollTimeout.count() },
  ss_{ Poco::Net::SocketAddress("0.0.0.0", port.value_) },
  workerThread_{ [this]{ this->workerLoop(); } }
{ }

TcpServer::~TcpServer()
{
  interrupt();
}

But::Optional<Log> TcpServer::readNextLog()
{
  return {};            // TODO
}

void TcpServer::interrupt()
{
  // TODO
}

void TcpServer::workerLoop()
{
  while(not quit_)
  {
    if( not ss_.poll(pollTimeout_, Poco::Net::Socket::SELECT_READ) )
      continue;
    Poco::Net::StreamSocket clientSocket = ss_.acceptConnection();
    Poco::Net::SocketStream client{clientSocket};
    nlohmann::json tmp;
    client >> tmp;
    queue_.withLock()->push_back( std::move(tmp) );
  }
}

}
