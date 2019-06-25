#include "LogATE/Net/TcpServer.hpp"
#include <chrono>

using Clock = std::chrono::system_clock;

namespace LogATE::Net
{

TcpServer::TcpServer(Utils::WorkerThreadsShPtr workers, const Port port):
  workers_{ std::move(workers) },
  server_{port},
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
  server_.interrupt();
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
      buffer_.resize(1*1024*1024);
      auto client = server_.accept();
      if(not client)
        continue;
      processClient(*client);
    }
    catch(...)
    {
      ++errors_;
      // what can we do aside from disconnection? :/
    }
  }
}


void TcpServer::processClient(Socket& socket)
{
  selector_.reset();
  while(not quit_)
  {
    const auto str = socket.readSome(buffer_);
    if( str.empty() )
      return;
    for(auto c: str)
    {
      try
      {
        selector_.update(c);
        if( not selector_.jsonComplete() )
          continue;
        // TODO: use thread pool for actual parsing!

        auto str = selector_.str();
        selector_.reset();
        auto json = nlohmann::json::parse( std::move(str) );
        const auto ptr = new AnnotatedLog{ std::move(json) };
        while( not queue_.push(ptr) ) { }
      }
      catch(...)
      {
        selector_.reset();
        ++errors_;
        // ignore any parse erorrs. if stream is disconnected, this will be detected next time loop condition is checked.
      }
    }
  }
}

}
