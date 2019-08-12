#pragma once
#include "LogATE/Net/TcpServer.hpp"
#include "LogATE/Tree/Node.hpp"
#include "CursATE/Curses/Init.hpp"
#include "CursATE/Screen/LogList.hpp"
#include "CursATE/Config.hpp"
#include <But/Threading/JoiningThread.hpp>
#include <atomic>


namespace CursATE
{

class Main final
{
public:
  explicit Main(Config const& config);
  ~Main();

  Main(Main const&) = delete;
  Main& operator=(Main const&) = delete;
  Main(Main&&) = delete;
  Main& operator=(Main&&) = delete;

  void run();
  void stop();

private:
  void dataPumpLoop();

  const CursATE::Curses::Init init_;
  const LogATE::Utils::WorkerThreadsShPtr workers_{ But::makeSharedNN<LogATE::Utils::WorkerThreads>() };
  LogATE::Net::TcpServer server_;
  Screen::LogList logList_;
  LogATE::Tree::NodeShPtr root_;
  std::atomic<bool> quit_{false};
  std::vector<But::Threading::JoiningThread<std::thread>> dataPumpThreads_;
};

}
