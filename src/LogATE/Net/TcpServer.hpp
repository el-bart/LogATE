#pragma once
#include "LogATE/Log.hpp"
#include "LogATE/Net/Port.hpp"
#include "LogATE/Net/Server.hpp"
#include "LogATE/Utils/WorkerThreads.hpp"
#include "LogATE/Json/Selector.hpp"
#include <But/Threading/JoiningThread.hpp>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Timespan.h>
#include <boost/lockfree/queue.hpp>
#include <chrono>
#include <thread>
#include <atomic>

namespace LogATE::Net
{

class TcpServer final: public Server
{
public:
  TcpServer(Utils::WorkerThreadsShPtr workers, Port port);
  TcpServer(Utils::WorkerThreadsShPtr workers, Port port, std::chrono::milliseconds pollTimeout);
  ~TcpServer();

  But::Optional<AnnotatedLog> readNextLog() override;
  void interrupt() override;
  size_t errors() const override { return errors_; }

private:
  void clearQueue();
  void workerLoop();
  void processClient(Poco::Net::StreamSocket clientSocket);

  using Queue = boost::lockfree::queue<AnnotatedLog*>;

  const Poco::Timespan pollTimeout_;
  std::atomic<size_t> errors_{0};
  std::atomic<bool> quit_{false};
  Json::Selector selector_;
  Queue queue_{2'000};
  Utils::WorkerThreadsShPtr workers_;
  Poco::Net::ServerSocket ss_;
  But::Threading::JoiningThread<std::thread> workerThread_;
};

}
