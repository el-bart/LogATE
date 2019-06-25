#include <cerrno>
#include <cstring>
#include <cassert>
#include <sys/epoll.h>
#include "LogATE/Net/epoll.hpp"
#include "LogATE/Net/detail/helpers.hpp"

using But::System::Descriptor;

namespace LogATE::Net
{

namespace
{
void registerDescriptors(But::System::Descriptor const& epollFd, std::initializer_list<ObservedEntry> fds)
{
  for (auto& e : fds)
  {
    epoll_event ev;
    ev.events = EPOLLRDHUP;
    switch (e.readyFor)
    {
      case ReadyFor::Read:
        ev.events |= EPOLLIN;
        break;
      case ReadyFor::Write:
        ev.events |= EPOLLOUT;
        break;
      case ReadyFor::ReadWrite:
        ev.events |= EPOLLIN | EPOLLOUT;
        break;
    }
    ev.data.ptr = e.fd;
    errno = 0;
    if (epoll_ctl(epollFd.get(), EPOLL_CTL_ADD, e.fd->get(), &ev) != 0)
      BUT_THROW(EpollFailed, "epoll_ctl() adding failed: " << strerror(errno));
  }
}

Descriptor* waitForEvent(But::System::Descriptor const& epollFd)
{
  epoll_event ev;
  constexpr auto maxEvents = 1;
  constexpr auto noTimeout = -1;
  const auto ret = detail::sysCallWrapper([&] { return epoll_wait(epollFd.get(), &ev, maxEvents, noTimeout); });
  if (ret == -1)
    BUT_THROW(EpollFailed, "epoll_wait() failed: " << strerror(errno));
  if (ret == 0)
    return nullptr;
  if (ret == 1)
    return reinterpret_cast<Descriptor*>(ev.data.ptr);
  assert(!"epoll_wait() returned unknown value - update the code!");
  throw std::logic_error{"epoll_wait() returned unknown value..."};
}
}


Descriptor* epoll(std::initializer_list<ObservedEntry> oe)
{
  if (oe.size() == 0u)
    return nullptr;
  errno = 0;
  Descriptor epollFd{epoll_create1(0)};
  if (not epollFd.opened())
    BUT_THROW(EpollFailed, "epoll_create1() failed: " << strerror(errno));

  registerDescriptors(epollFd, oe);
  return waitForEvent(epollFd);
}

}
