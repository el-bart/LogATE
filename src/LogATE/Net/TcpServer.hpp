#pragma once
#include "LogATE/Log.hpp"
#include "LogATE/Net/Port.hpp"
#include "LogATE/Net/Server.hpp"
#include "LogATE/Net/Socket.hpp"
#include "LogATE/Net/detail/TcpServerImpl.hpp"
#include "LogATE/Utils/WorkerThreads.hpp"
#include "LogATE/Tree/Path.hpp"
#include <But/Threading/Fifo.hpp>
#include <But/Threading/JoiningThread.hpp>
#include <chrono>
#include <thread>
#include <atomic>

namespace LogATE::Net
{

class TcpServer final: public Server
{
public:
  BUT_DEFINE_EXCEPTION(InvalidKeyPath, But::Exception, "invalid key path");

  enum class JsonParsingMode
  {
    ParseToEndOfJson,
    HardBreakOnNewLine
  };

  TcpServer(Utils::WorkerThreadsShPtr workers,
            Port port,
            Tree::Path keyPath,
            JsonParsingMode jsonParsingMode,
            std::chrono::milliseconds bulkPackageTimeout = std::chrono::milliseconds{500});
  ~TcpServer();

  std::vector<AnnotatedLog> readNextLogs() override;
  void interrupt() override;
  size_t errors() const override { return *errors_; }

private:
  using Clock = std::chrono::steady_clock;
  using Queue = But::Threading::Fifo<std::vector<AnnotatedLog>>;

  void workerLoop();
  template<typename Selector>
  void processClient(Socket& socket);
  void sendOutRemainingLogs(std::vector<std::string>&& jsons);
  bool waitForQueueSizeLowEnough();
  void queueJsonsForParsing(std::vector<std::string>& jsons);
  template<typename Selector>
  bool processInputData(Selector& selector, std::vector<std::string>& inputJsons, std::string_view const& str, Clock::time_point deadline);
  bool processInputIfReady(std::vector<std::string>& inputJsons, Clock::time_point deadline);
  void processOneClient(LogATE::Net::Socket client);
  void cleanupDeadThreads();

  const JsonParsingMode jsonParsingMode_;
  const std::chrono::milliseconds bulkPackageTimeout_;
  const Tree::Path keyPath_;
  But::NotNullShared<std::atomic<size_t>> errors_{ But::makeSharedNN<std::atomic<size_t>>(0) };
  But::NotNullShared<std::atomic<bool>> quit_{ But::makeSharedNN<std::atomic<bool>>(false) };
  But::NotNullShared<Queue> queue_{ But::makeSharedNN<Queue>() };
  Utils::WorkerThreadsShPtr workers_;
  detail::TcpServerImpl server_;
  std::vector<But::Threading::JoiningThread<std::thread>> clientThreads_;
  But::Threading::JoiningThread<std::thread> workerThread_;
};

}
