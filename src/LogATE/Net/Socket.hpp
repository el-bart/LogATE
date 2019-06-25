#pragma once
#include <string>
#include <But/System/Descriptor.hpp>
#include "LogATE/Net/SocketDescriptorPair.hpp"
#include "LogATE/Net/Exception.hpp"
#include "LogATE/Net/epoll.hpp"

namespace LogATE::Net
{

class Socket final
{
public:
  BUT_DEFINE_EXCEPTION(Error, Exception, "socket error");

  explicit Socket(But::System::Descriptor sock);
  ~Socket() = default;

  Socket(Socket const&) = delete;
  Socket& operator=(Socket const&) = delete;

  Socket(Socket&&) = default;
  Socket& operator=(Socket&&) = default;

  /** @brief interrupts a blocking read()/write() call. can be called from a different thread.
   */
  void interrupt();

  std::string_view read(std::string& buffer);
  std::string_view readSome(std::string& buffer);
  size_t write(std::string const& data);

private:
  bool waitForData(ReadyFor op);

  But::System::Descriptor sock_;
  SocketDescriptorPair sdp_;
};

}
