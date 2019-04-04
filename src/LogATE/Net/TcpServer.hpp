#pragma once
#include "LogATE/Log.hpp"
#include "LogATE/Net/Port.hpp"
#include "LogATE/Net/Server.hpp"
#include <But/Threading/Fifo.hpp>
#include <But/Threading/JoiningThread.hpp>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Timespan.h>
#include <chrono>
#include <thread>
#include <atomic>

namespace LogATE::Net
{

class TcpServer final: public Server
{
public:
  explicit TcpServer(Port port);
  TcpServer(Port port, std::chrono::milliseconds pollTimeout);
  ~TcpServer();

  But::Optional<Log> readNextLog() override;
  void interrupt() override;
  size_t errors() const override { return errors_; }

private:
  void workerLoop();
  void processClient(Poco::Net::StreamSocket clientSocket);

  using Queue = But::Threading::Fifo<But::Optional<Log>>;

  const Poco::Timespan pollTimeout_;
  std::atomic<size_t> errors_{0};
  std::atomic<bool> quit_{false};
  Queue queue_;
  Poco::Net::ServerSocket ss_;
  But::Threading::JoiningThread<std::thread> workerThread_;
};

}
