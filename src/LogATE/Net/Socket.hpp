#pragma once
#include <string>
#include <iosfwd>
#include <But/System/Descriptor.hpp>
#include "LogATE/Net/SocketDescriptorPair.hpp"
#include "LogATE/Net/Exception.hpp"
#include "LogATE/Net/epoll.hpp"
#include "LogATE/Net/detail/WaitResult.hpp"

namespace LogATE::Net
{

class Socket final
{
public:
  enum class Reason
  {
    Ok,
    NoData,
    Timeout,
    Interrupted,
    ClosedByRemoteEnd
  };

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

  std::pair<Reason, std::string_view> read(std::string& buffer);
  std::pair<Reason, std::string_view> readSome(std::string& buffer);
  std::pair<Reason, std::string_view> readSome(std::string& buffer, std::chrono::milliseconds timeout);
  size_t write(std::string const& data);

private:
  detail::WaitResult waitForData(ReadyFor op);
  detail::WaitResult waitForData(ReadyFor op, std::chrono::milliseconds timeout);

  template<typename ...Args>
  std::pair<Socket::Reason, std::string_view> readSomeImpl(std::string& buffer, Args const&...);

  But::System::Descriptor sock_;
  SocketDescriptorPair sdp_;
};


std::ostream& operator<<(std::ostream& os, Socket::Reason reason);

}
