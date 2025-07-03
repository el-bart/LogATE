#include "LogATE/Net/TcpServer.hpp"
#include <signal.h>
#include <memory>
#include <iostream>

using LogATE::Tree::KeyExtractor;

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
  const auto workers = But::makeSharedNN<LogATE::Utils::WorkerThreads>();
  const auto parseMode = LogATE::Net::TcpServer::JsonParsingMode::ParseToEndOfJson;
  const auto keyExtractor = But::makeSharedNN<KeyExtractor>( LogATE::Tree::Path::parse(".But::PreciseDT"), KeyExtractor::SourceFormat::ISO8601_ns );
  auto server = std::make_shared<LogATE::Net::TcpServer>(workers, LogATE::Net::Port{6666}, keyExtractor, parseMode);
  g_serverPtr = server;
  for(auto sig: {SIGTERM, SIGHUP, SIGINT})
    if( signal(sig, signalHandler) == SIG_ERR )
      throw std::runtime_error{"failed to register server"};

  while(true)
  {
    const auto logs = server->readNextLogs();
    if( logs.empty() )
      break;
    for(auto& log: logs)
      std::cout << log.log().sequenceNumber().value_ << ": " << log.json() << std::endl;
  }
}
