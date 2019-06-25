#pragma once
#include <cerrno>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "LogATE/Net/Exception.hpp"
#include "LogATE/Net/epoll.hpp"

namespace LogATE::Net::detail
{

template <typename F>
auto sysCallWrapper(F&& f)
{
  decltype(f()) tmp;
  do
  {
    errno = 0;
    tmp = f();
  } while (tmp == -1 && errno == EINTR);
  return tmp;
}

BUT_DEFINE_EXCEPTION(SocketSetupError, Exception, "fcntl() failed");

inline auto fcntlGet(const int fd)
{
  const auto flags = sysCallWrapper([&] { return ::fcntl(fd, F_GETFL, 0); });
  if (flags == -1)
    BUT_THROW(SocketSetupError, "getting options: " << strerror(errno));
  return flags;
}

inline void fcntlSet(const int fd, const int flags)
{
  if (sysCallWrapper([&] { return ::fcntl(fd, F_SETFL, flags); }) == -1)
    BUT_THROW(SocketSetupError, "setting options: " << strerror(errno));
}

inline void makeNonBlocking(const int fd)
{
  fcntlSet(fd, fcntlGet(fd) | O_NONBLOCK);
}
inline void makeBlocking(const int fd)
{
  fcntlSet(fd, fcntlGet(fd) & ~O_NONBLOCK);
}

inline void allowAddressReuse(const int fd)
{
  if (sysCallWrapper([=] { const auto flag = 1; return ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)); }) == -1)
    BUT_THROW(SocketSetupError, "setsockopt(SO_REUSEPORT): " << strerror(errno));
}

inline void ignoreDataFromSocket(const int fd)
{
  char junk[4*1024];
  while (sysCallWrapper([&] { return ::read(fd, junk, sizeof(junk)); }) > 0)
  { }
}

inline bool waitForData(But::System::Descriptor& interruptSource, const ReadyFor op, But::System::Descriptor& socket)
{
  const auto fd = ::LogATE::Net::epoll({{&interruptSource, ReadyFor::Read}, {&socket, op}});
  if (fd != &interruptSource)
    return true;
  ignoreDataFromSocket(interruptSource.get());
  return false;
}

}
