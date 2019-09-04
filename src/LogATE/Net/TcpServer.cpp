#include "LogATE/Net/TcpServer.hpp"
#include "LogATE/Json/Selector.hpp"
#include <chrono>

namespace LogATE::Net
{

auto pathCheck(Tree::Path keyPath)
{
  if( keyPath.empty() )
    BUT_THROW(TcpServer::InvalidKeyPath, "key path cannot be empty");
  if( not keyPath.absolute() )
    BUT_THROW(TcpServer::InvalidKeyPath, "key path must be absolute: " << keyPath.str() );
  return keyPath;
}

TcpServer::TcpServer(Utils::WorkerThreadsShPtr workers,
                     const Port port,
                     Tree::Path keyPath,
                     const JsonParsingMode jsonParsingMode,
                     std::chrono::milliseconds bulkPackageTimeout):
  jsonParsingMode_{jsonParsingMode},
  bulkPackageTimeout_{bulkPackageTimeout},
  keyPath_{ pathCheck(std::move(keyPath)) },
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

      switch(jsonParsingMode_)
      {
        case JsonParsingMode::ParseToEndOfJson:
             processClient<Json::Selector>(*client);
             break;
        case JsonParsingMode::HardBreakOnNewLine:
             processClient<Json::NewLineSplit>(*client);
             break;
      }
    }
    catch(...)
    {
      ++*errors_;
      // what can we do aside from disconnection? :/
    }
  }
}


template<typename Selector>
void TcpServer::processClient(Socket& socket)
{
  Selector selector;
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
    if( processInputData(selector, inputJsons, ret.second, deadline) )
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
  Queue::lock_type lock{*queue_};
  while( not queue_->waitForSizeBelow(750'000, lock, std::chrono::seconds{1}) )
    if(*quit_)
      return false;
  return true;
}


void TcpServer::queueJsonsForParsing(std::vector<std::string>& jsons)
{
  std::vector<std::string> tmp;
  tmp.reserve( jsons.size() );
  tmp.swap(jsons);

  if( not waitForQueueSizeLowEnough() )
    return;

  workers_->enqueueBatch( [queue = queue_, c = std::move(tmp), quit = quit_, keyPath = keyPath_, errors = errors_] {
    for(auto&& str: std::move(c))
    {
      try
      {
        if(*quit)
          return;
        auto log = AnnotatedLog{ std::move(str), keyPath };   // TODO: sequence number should be preserved from original input...
        auto opt = But::Optional<AnnotatedLog>{ std::move(log) };
        Queue::lock_type lock{*queue};
        queue->push( std::move(opt) );
      }
      catch(...)
      {
        ++*errors;
        // incorrect JSON may get accepted by a selector - this will detect this has happened
      }
    }
  } );
}


template<typename Selector>
bool TcpServer::processInputData(Selector& selector,
                                 std::vector<std::string>& inputJsons,
                                 std::string_view const& str,
                                 const Clock::time_point deadline)
{
  auto dataSent = false;
  for(auto c: str)
  {
    try
    {
      selector.update(c);
      if( not selector.jsonComplete() )
        continue;
      inputJsons.push_back( std::string{ selector.str() } );
      selector.reset();

      if( processInputIfReady(inputJsons, deadline) )
        dataSent = true;
    }
    catch(...)
    {
      selector.reset();
      ++*errors_;
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
