#include <cerrno>
#include <cstring>
#include <cassert>
#include <sys/epoll.h>
#include <But/assert.hpp>
#include "LogATE/Net/epoll.hpp"
#include "LogATE/Net/detail/helpers.hpp"

using But::System::Descriptor;

namespace LogATE::Net
{

namespace
{
void registerDescriptors(But::System::Descriptor const& epollFd, std::initializer_list<ObservedEntry> fds)
{
  for(auto& e: fds)
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
    if( epoll_ctl(epollFd.get(), EPOLL_CTL_ADD, e.fd->get(), &ev) != 0 )
      BUT_THROW(EpollFailed, "epoll_ctl() adding failed: " << strerror(errno));
  }
}

Descriptor* waitForEvent(But::System::Descriptor const& epollFd, const int timeout)
{
  epoll_event ev;
  constexpr auto maxEvents = 1;
  const auto ret = detail::sysCallWrapper( [&] { return epoll_wait(epollFd.get(), &ev, maxEvents, timeout); } );
  if(ret == -1)
    BUT_THROW(EpollFailed, "epoll_wait() failed: " << strerror(errno));
  if(ret == 0)      // timeout
    return nullptr;
  if(ret == 1)
    return reinterpret_cast<Descriptor*>(ev.data.ptr);
  BUT_ASSERT(!"epoll_wait() returned unknown value - update the code!");
  throw std::logic_error{"epoll_wait() returned unknown value..."};
}

Descriptor* epollImpl(std::initializer_list<ObservedEntry> oe, const int timeout)
{
  if( oe.size() == 0u )
    return nullptr;
  errno = 0;
  Descriptor epollFd{epoll_create1(0)};
  if( not epollFd.opened() )
    BUT_THROW(EpollFailed, "epoll_create1() failed: " << strerror(errno));

  registerDescriptors(epollFd, oe);
  return waitForEvent(epollFd, timeout);
}
}


Descriptor* epoll(std::initializer_list<ObservedEntry> oe)
{
  const auto noTimeout = -1;
  return epollImpl(oe, noTimeout);
}


Descriptor* epoll(std::initializer_list<ObservedEntry> oe, std::chrono::milliseconds timeout)
{
  return epollImpl( oe, timeout.count() );
}

}
