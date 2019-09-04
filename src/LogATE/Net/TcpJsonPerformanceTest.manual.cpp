#include "LogATE/Net/TcpServer.hpp"
#include "LogATE/Net/TcpRawClient.hpp"
#include <But/Threading/JoiningThread.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using Clock = std::chrono::system_clock;
using Thread = But::Threading::JoiningThread<std::thread>;


int main()
{
  const auto port = LogATE::Net::Port{6666};
  const auto workers = But::makeSharedNN<LogATE::Utils::WorkerThreads>();
  const auto parseMode = LogATE::Net::TcpServer::JsonParsingMode::ParseToEndOfJson;
  const auto keyPath = LogATE::Tree::Path::parse(".But::PreciseDT");
  LogATE::Net::TcpServer server{workers, port, keyPath, parseMode};
  LogATE::Net::TcpRawClient client{"127.0.0.1", port};
  const auto N = 1'000'000u;

  const auto testLog = std::string{
#if 1
                             R"({
                                "PING": {
                                  "PONG": {
                                    "narf": {
                                      "fran": "a_c"
                                    }
                                  }
                                },
                                "foo": {
                                  "bar": "a/c"
                                },
                                "array": [
                                  { "one": 1 },
                                  { "two": 2 }
                                ]
                              })"
#else
                             R"({
                                "one": {
                                  "PING": {
                                    "PONG": {
                                      "narf": 42
                                    }
                                  }
                                },
                                "two": {
                                  "PING": {
                                    "PONG": {
                                      "narf": {
                                        "fran": "aaa"
                                      }
                                    }
                                  }
                                },
                                "three": {
                                  "foo": {
                                    "bar": "xxx"
                                  }
                                },
                                "four": {
                                  "foo": {
                                    "bar": "yyy"
                                  }
                                },
                                "five": {
                                  "foo": [
                                    { "one": 1 },
                                    { "two": 2 }
                                  ]
                                }
                              })"
#endif
    }; // string

  Thread thClient{ [&] { for(auto i=0u; i<N; ++i) client.write(testLog); } };

  const auto printDelay = std::chrono::seconds{1};
  auto deadline = Clock::now();
  for(auto i=0u; i<N;)
  {
    i += server.readNextLogs().size();
    const auto now = Clock::now();
    if( now > deadline )
    {
      deadline += printDelay;
      std::cout << i << " / " << N << std::endl;
    }
  }
}
