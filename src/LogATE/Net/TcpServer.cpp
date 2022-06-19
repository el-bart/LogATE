#include "LogATE/Net/TcpServer.hpp"
#include "LogATE/Json/Selector.hpp"
#include <chrono>

namespace LogATE::Net
{

namespace
{
constexpr auto g_jsonChunkSize = 1'000u;
constexpr auto g_maxElementsInQueue = 500u;
}


auto pathCheck(Tree::Path keyPath)
{
  if( keyPath.empty() )
    BUT_THROW(TcpServer::InvalidKeyPath, "key path cannot be empty");
  if( not keyPath.isAbsolute() )
    BUT_THROW(TcpServer::InvalidKeyPath, "key path must be absolute: " << keyPath.str() );
  if( not keyPath.isUnique() )
    BUT_THROW(TcpServer::InvalidKeyPath, "key path must be unique (no wildcards allowed): " << keyPath.str() );
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
}


std::vector<AnnotatedLog> TcpServer::readNextLogs()
{
  Queue::lock_type lock{*queue_};
  queue_->waitForNonEmpty(lock);
  auto logs = std::move( queue_->top() );
  queue_->pop();
  return logs;
}


void TcpServer::interrupt()
{
  *quit_ = true;
  server_.interrupt();
  {
    Queue::lock_type lock{*queue_};
    queue_->push( Queue::value_type{} );
  }
}


void TcpServer::workerLoop()
{
  while(not *quit_)
  {
    try
    {
      auto client = server_.accept();
      if(not client)
        continue;

      clientThreads_.emplace_back(&TcpServer::processOneClient, this, std::move(*client) );
      cleanupDeadThreads();
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
  std::string buffer;
  buffer.resize(1*1024*1024);

  Selector selector;
  std::vector<std::string> inputJsons;
  auto deadline = Clock::now() + bulkPackageTimeout_;
  while(not *quit_)
  {
    const auto ret = socket.readSome(buffer, bulkPackageTimeout_);
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


namespace
{
auto parseToAnnotatedLogs(std::vector<std::string>&& jsons, Tree::Path const& keyPath, std::atomic<uint64_t>& errors)
{
  std::vector<AnnotatedLog> out;
  out.reserve( jsons.size() );
  for(auto& str: jsons)
  {
    try
    {
      auto log = AnnotatedLog{ std::move(str), keyPath };
      out.push_back( std::move(log) );
    }
    catch(...)
    {
      // incorrect JSON may get accepted by a selector - this will detect this has happened
      ++errors;
    }
  }
  return out;
}
}


void TcpServer::sendOutRemainingLogs(std::vector<std::string>&& jsons)
{
  if(*quit_)
    return;
  if( jsons.empty() )
    return;
  auto logs = parseToAnnotatedLogs( std::move(jsons), keyPath_, *errors_ );
  Queue::lock_type lock{*queue_};
  queue_->push( std::move(logs) );
}


bool TcpServer::waitForQueueSizeLowEnough()
{
  Queue::lock_type lock{*queue_};
  while( not queue_->waitForSizeBelow(g_maxElementsInQueue, lock, std::chrono::seconds{1}) )
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

  workers_->enqueueBatch( [queue = queue_, c = std::move(tmp), quit = quit_, keyPath = keyPath_, errors = errors_] () mutable {
      if(*quit)
        return;
      if( c.empty() )
        return;
      auto logs = parseToAnnotatedLogs( std::move(c), keyPath, *errors );
      Queue::lock_type lock{*queue};
      queue->push( std::move(logs) );
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
  if( inputJsons.size() < g_jsonChunkSize && Clock::now() < deadline )
    return false;

  if( not waitForQueueSizeLowEnough() )
    return false;
  queueJsonsForParsing(inputJsons);
  return true;
}


void TcpServer::processOneClient(LogATE::Net::Socket client)
{
  try
  {
    switch(jsonParsingMode_)
    {
      case JsonParsingMode::ParseToEndOfJson:
           processClient<Json::Selector>(client);
           break;
      case JsonParsingMode::HardBreakOnNewLine:
           processClient<Json::NewLineSplit>(client);
           break;
    }
  }
  catch(...)
  {
    ++*errors_;
    // what can we do aside from disconnection? :/
  }
}


void TcpServer::cleanupDeadThreads()
{
    // TODO...
}

}
