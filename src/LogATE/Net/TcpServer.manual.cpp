#include "LogATE/Net/TcpServer.hpp"
#include <signal.h>
#include <memory>
#include <iostream>

std::weak_ptr<LogATE::Net::TcpServer> g_serverPtr;

void signalHandler(int sig)
{
  std::cerr << std::endl << "RECEIVED SIGNAL " << sig << std::endl;
  const auto ptr = g_serverPtr.lock();
  if(not ptr)
    return;
  std::cerr << "stopping server..." << std::endl;
  ptr->interrupt(); // note: it's not actually valid, as this call locks! just for playground purposes.
}

int main()
{
  auto server = std::make_shared<LogATE::Net::TcpServer>(LogATE::Net::Port{6666});
  g_serverPtr = server;
  for(auto sig: {SIGTERM, SIGHUP, SIGINT})
    if( signal(sig, signalHandler) == SIG_ERR )
      throw std::runtime_error{"failed to register server"};

  while(true)
  {
    const auto log = server->readNextLog();
    if(not log)
      break;
    std::cout << log->log().sequenceNumber().value_ << ": " << log->json() << std::endl;
  }
}
