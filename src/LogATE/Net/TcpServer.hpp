#pragma once
#include "LogATE/Log.hpp"
#include "LogATE/Net/Port.hpp"
#include "LogATE/Net/Server.hpp"
#include "LogATE/Net/Socket.hpp"
#include "LogATE/Net/detail/TcpServerImpl.hpp"
#include "LogATE/Utils/WorkerThreads.hpp"
#include "LogATE/Json/Selector.hpp"
#include <But/Threading/Fifo.hpp>
#include <But/Threading/JoiningThread.hpp>
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
  ~TcpServer();

  But::Optional<AnnotatedLog> readNextLog() override;
  void interrupt() override;
  size_t errors() const override { return errors_; }

private:
  void workerLoop();
  void processClient(Socket& socket);

  //using Queue = boost::lockfree::queue<AnnotatedLog*>;
  using Queue = But::Threading::Fifo<But::Optional<AnnotatedLog>>;

  std::atomic<size_t> errors_{0};
  But::NotNullShared<std::atomic<bool>> quit_{ But::makeSharedNN<std::atomic<bool>>(false) };
  Json::Selector selector_;
  Queue queue_;
  Utils::WorkerThreadsShPtr workers_;
  detail::TcpServerImpl server_;
  std::string buffer_;
  But::Threading::JoiningThread<std::thread> workerThread_;
};

}
