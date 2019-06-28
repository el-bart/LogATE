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


But::Optional<AnnotatedLog> TcpServer::readNextLog()
{
  Queue::lock_type lock{queue_};
  queue_.waitForNonEmpty(lock);
  auto v = std::move( queue_.top() );
  queue_.pop();
  return v;
}


void TcpServer::interrupt()
{
  *quit_ = true;
  server_.interrupt();
  {
    Queue::lock_type lock{queue_};
    queue_.push( But::Optional<AnnotatedLog>{} );
  }
}


void TcpServer::workerLoop()
{
  while(not *quit_)
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
  std::vector<std::string> inputJsons;
  while(not *quit_)
  {
    const auto str = socket.readSome(buffer_);
    if( str.empty() )
    {
      for(auto&& str: std::move(inputJsons))
      {
        if(*quit_)
          return;
        auto log = AnnotatedLog{ std::move(str) };
        auto opt = But::Optional<AnnotatedLog>{ std::move(log) };
        Queue::lock_type lock{queue_};
        queue_.push( std::move(opt) );
      }
      return;
    }

    for(auto c: str)
    {
      try
      {
        selector_.update(c);
        if( not selector_.jsonComplete() )
          continue;
        inputJsons.push_back( std::string{ selector_.str() } );
        selector_.reset();

        if( inputJsons.size() < 1'000 )    // TODO + timeout for life preview!
          continue;
        std::vector<std::string> tmp;
        tmp.reserve( inputJsons.size() );
        tmp.swap(inputJsons);

        while( workers_->running() >= 10*workers_->threads() )
        {
          if(*quit_)
            return;
          std::this_thread::sleep_for(std::chrono::milliseconds{100});
        }
        auto fut = workers_->enqueue( [&, c = std::move(tmp), quit = quit_] {
            for(auto&& str: std::move(c))
            {
              if(*quit)
                return;
              auto log = AnnotatedLog{ std::move(str) };    // TODO: sequence number should be preserved from original input...
              auto opt = But::Optional<AnnotatedLog>{ std::move(log) };
              Queue::lock_type lock{queue_};
              while( not queue_.waitForSizeBelow(2'000, lock, std::chrono::seconds{1}) )
                if(*quit)
                  return;
              queue_.push( std::move(opt) );
            }
        } );
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
