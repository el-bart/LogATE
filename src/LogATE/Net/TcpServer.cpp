#include "LogATE/Net/TcpServer.hpp"
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/Socket.h>
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
  Queue::lock_type lock{queue_};
  if( queue_.empty() )
    queue_.waitForNonEmpty(lock);
  auto tmp = std::move( queue_.top() );
  queue_.pop();
  return tmp;
}

void TcpServer::interrupt()
{
  quit_ = true;
  queue_.withLock()->push( Queue::value_type{} );
}

void TcpServer::workerLoop()
{
  while(not quit_)
  {
    try
    {
      if( not ss_.poll(pollTimeout_, Poco::Net::Socket::SELECT_READ) )
        continue;
      processClient( ss_.acceptConnection() );
    }
    catch(...)
    {
      ++errors_;
      // what can we do aside from disconnection? :/
    }
  }
}

namespace
{
auto isStreamUsable(std::ostream const& os)
{
  if( os.bad() || os.fail() )
    return false;
  if( os.eof() )
    return false;
  return os.good();
}
}

void TcpServer::processClient(Poco::Net::StreamSocket clientSocket)
{
  Poco::Net::SocketStream clientStream{clientSocket};
  nlohmann::json tmp;
  while( not quit_ && isStreamUsable(clientStream) )
  {
    try
    {
      clientStream >> tmp;
      queue_.withLock()->push( makeLog( std::move(tmp) ) );
    }
    catch(...)
    {
      ++errors_;
      // ignore any parse erorrs. if stream is disconnected, this will be detected next time loop condition is checked.
    }
  }
}

}
