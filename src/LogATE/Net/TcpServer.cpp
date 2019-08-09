#include "LogATE/Net/TcpServer.hpp"
#include <chrono>

namespace LogATE::Net
{

TcpServer::TcpServer(Utils::WorkerThreadsShPtr workers,
                     const Port port,
                     std::chrono::milliseconds bulkPackageTimeout):
  bulkPackageTimeout_{bulkPackageTimeout},
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
  Queue::lock_type lock{*queue_};
  queue_->waitForNonEmpty(lock);
  auto v = std::move( queue_->top() );
  queue_->pop();
  return v;
}


void TcpServer::interrupt()
{
  *quit_ = true;
  server_.interrupt();
  {
    Queue::lock_type lock{*queue_};
    queue_->push( But::Optional<AnnotatedLog>{} );
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
  auto deadline = Clock::now() + bulkPackageTimeout_;
  while(not *quit_)
  {
    const auto ret = socket.readSome(buffer_, bulkPackageTimeout_);
    switch(ret.first)
    {
      case Socket::Reason::Ok:
           break;

      case Socket::Reason::Timeout:
      case Socket::Reason::NoData:
           if( processInputIfReady(inputJsons, deadline) )
             deadline += bulkPackageTimeout_;
           break;

      case Socket::Reason::Interrupted:
      case Socket::Reason::ClosedByRemoteEnd:
           sendOutRemainingLogs( std::move(inputJsons) );
           return;
    }
    if( processInputData(inputJsons, ret.second, deadline) )
      deadline += bulkPackageTimeout_;
  }
}


void TcpServer::sendOutRemainingLogs(std::vector<std::string>&& jsons)
{
  for(auto&& str: std::move(jsons))
  {
    if(*quit_)
      return;
    auto log = AnnotatedLog{ std::move(str), keyPath_ };
    auto opt = But::Optional<AnnotatedLog>{ std::move(log) };
    Queue::lock_type lock{*queue_};
    queue_->push( std::move(opt) );
  }
}


bool TcpServer::waitForQueueSizeLowEnough()
{
  const auto maxRunning = std::min( 2u*workers_->threads(), 10lu ); // TODO: limitation can be lifted when locking contingention is solved
  while( workers_->running() >= maxRunning )
  {
    if(*quit_)
      return false;
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
  }
  return true;
}


void TcpServer::queueJsonsForParsing(std::vector<std::string>& jsons)
{
  std::vector<std::string> tmp;
  tmp.reserve( jsons.size() );
  tmp.swap(jsons);

  if( not waitForQueueSizeLowEnough() )
    return;

  workers_->enqueue( [queue = queue_, c = std::move(tmp), quit = quit_, keyPath = keyPath_] {
    for(auto&& str: std::move(c))
    {
      if(*quit)
        return;
      auto log = AnnotatedLog{ std::move(str), keyPath };   // TODO: sequence number should be preserved from original input...
      auto opt = But::Optional<AnnotatedLog>{ std::move(log) };
      Queue::lock_type lock{*queue};
      while( not queue->waitForSizeBelow(750'000, lock, std::chrono::seconds{1}) )
        if(*quit)
          return;
      queue->push( std::move(opt) );
    }
  } );
}


bool TcpServer::processInputData(std::vector<std::string>& inputJsons, std::string_view const& str, const Clock::time_point deadline)
{
  auto dataSent = false;
  for(auto c: str)
  {
    try
    {
      selector_.update(c);
      if( not selector_.jsonComplete() )
        continue;
      inputJsons.push_back( std::string{ selector_.str() } );
      selector_.reset();

      if( processInputIfReady(inputJsons, deadline) )
        dataSent = true;
    }
    catch(...)
    {
      selector_.reset();
      ++errors_;
      // ignore any parse erorrs. if stream is disconnected, this will be detected next time loop condition is checked.
    }
  }
  return dataSent;
}


bool TcpServer::processInputIfReady(std::vector<std::string>& inputJsons, const Clock::time_point deadline)
{
  if( inputJsons.size() < 1'000 && Clock::now() < deadline )
    return false;

  if( not waitForQueueSizeLowEnough() )
    return false;
  queueJsonsForParsing(inputJsons);
  return true;
}

}
