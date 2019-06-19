#include "LogATE/Net/TcpServer.hpp"
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/Socket.h>
#include <Poco/Net/SocketStream.h>
#include <chrono>

using Clock = std::chrono::system_clock;

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
  if( workerThread_.joinable() )
    workerThread_.join();
  clearQueue();
}

namespace
{
auto updateBackOffTime(const std::chrono::milliseconds in)
{
  constexpr auto maxBackOffTime = std::chrono::milliseconds{500};
  const auto candidate = in + std::chrono::milliseconds{ std::max<unsigned>( in.count(), 1u ) * 2u };
  if( candidate > maxBackOffTime )
    return maxBackOffTime;
  return candidate;
}
}

But::Optional<AnnotatedLog> TcpServer::readNextLog()
{
  auto backOffTime = std::chrono::milliseconds{0};
  AnnotatedLog* ptr{nullptr};
  while( not queue_.pop(ptr) )
  {
    std::this_thread::sleep_for(backOffTime);
    backOffTime = updateBackOffTime(backOffTime);
  }
  if(not ptr)
    return {};
  std::unique_ptr<AnnotatedLog> uptr{ptr};
  return But::Optional<AnnotatedLog>{ std::move(*uptr) };
}

void TcpServer::interrupt()
{
  quit_ = true;
  auto empty = Queue::value_type{}; // explicit variable to bypass invalid GCC-8 warning
  while( not queue_.push(empty) ) { }
}

void TcpServer::clearQueue()
{
  while( not queue_.empty() )
  {
    AnnotatedLog* ptr{nullptr};
    queue_.pop(ptr);
    delete ptr;
  }
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
  while( not quit_ && isStreamUsable(clientStream) )
  {
    try
    {
      nlohmann::json tmp;
      clientStream >> tmp;  // TODO: how to interrupt this when server shutdown has been requested,
                            //       yet remote end is still connected but not transmitting atm?
      if( tmp.is_null() )
        continue;
      const auto ptr = new AnnotatedLog{ std::move(tmp) };
      while( not queue_.push(ptr) ) { }
    }
    catch(...)
    {
      ++errors_;
      // ignore any parse erorrs. if stream is disconnected, this will be detected next time loop condition is checked.
    }
  }
}

}
