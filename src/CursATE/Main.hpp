#pragma once
#include "LogATE/Net/Port.hpp"
#include "LogATE/Net/TcpServer.hpp"
#include "LogATE/Tree/Node.hpp"
#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/ScrolableWindow.hpp"       
#include <But/Threading/JoiningThread.hpp>
#include <atomic>


namespace CursATE
{

class Main final
{
public:
  explicit Main(LogATE::Net::Port port);
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
  LogATE::Tree::NodeShPtr root_;
  LogATE::Net::TcpServer server_;
  std::atomic<bool> quit_{false};
  But::Threading::JoiningThread<std::thread> dataPump_;
  Curses::ScrolableWindow win_;         
};

}
