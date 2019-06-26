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
}

namespace
{
template<typename Q>
auto getNext(Q& q)
{
  typename Q::lock_type lock{q};
  q.waitForNonEmpty(lock);
  auto v = std::move( q.top() );
  q.pop();
  return v;
}
}

But::Optional<AnnotatedLog> TcpServer::readNextLog()
{
  auto backOffTime = std::chrono::milliseconds{0};
  auto fut = getNext(queue_);
  return std::move( fut.get() );
}


void TcpServer::interrupt()
{
  quit_ = true;
  server_.interrupt();
  {
    std::promise< But::Optional<AnnotatedLog> > promise;
    promise.set_value( But::Optional<AnnotatedLog>{} );
    queue_.push( promise.get_future() );
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

        auto fut = workers_->enqueue( [&, str = std::string{ selector_.str() }] {
            auto log = AnnotatedLog{ std::move(str) };
            return But::Optional<AnnotatedLog>{ std::move(log) };
        } );

        selector_.reset();

        Queue::lock_type lock{queue_};
        queue_.waitForSizeBelow(10'000, lock);
        queue_.push( std::move(fut) );
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
