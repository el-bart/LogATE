#include <cstring>
#include <cassert>
#include <cerrno>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <But/assert.hpp>
#include "LogATE/Net/detail/TcpServerImpl.hpp"
#include "LogATE/Net/detail/helpers.hpp"

namespace LogATE::Net::detail
{

namespace
{
auto listenOnPort(const Port port)
{
  But::System::Descriptor sock{
    detail::sysCallWrapper(
        [] { return socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP); } ) };
  if( not sock.opened() )
    BUT_THROW(TcpServerImpl::Error, "socket() failed: " << strerror(errno));
  detail::allowAddressReuse(sock.get());

  sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port.value_);

  if( detail::sysCallWrapper( [&] { return bind(sock.get(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)); } ) == -1 )
    BUT_THROW(TcpServerImpl::Error, "bind(port=" << port.value_ << ") failed: " << strerror(errno));

  constexpr auto queueLen = 2;
  if( detail::sysCallWrapper( [&] { return listen(sock.get(), queueLen); } ) == -1 )
    BUT_THROW(TcpServerImpl::Error, "listen() failed: " << strerror(errno));

  return sock;
}
}


TcpServerImpl::TcpServerImpl(const Port port) : sock{listenOnPort(port)}
{
  BUT_ASSERT(sock.opened());
  detail::makeNonBlocking(sdp.first.get());
  detail::makeNonBlocking(sdp.second.get());
}


void TcpServerImpl::interrupt()
{
  if( detail::sysCallWrapper( [&] { return ::write(sdp.first.get(), "x", 1); } ) != 1 )
    BUT_THROW(TcpServerImpl::Error, "interruption failed - write(): " << strerror(errno));
}


But::Optional<Socket> TcpServerImpl::accept()
{
  if( not waitForConnection() )
    return {};
  sockaddr addr;
  auto len = socklen_t{0};
  bzero(&addr, sizeof(addr));
  But::System::Descriptor fd{ detail::sysCallWrapper( [&]{ return ::accept(sock.get(), &addr, &len); } ) };
  if( not fd.opened() )
    BUT_THROW(TcpServerImpl::Error, "accept(): " << strerror(errno));
  return But::Optional<Socket>{Socket{std::move(fd)}};
}


bool TcpServerImpl::waitForConnection()
{
  const auto ret = detail::waitForData(sdp.second, ReadyFor::Read, sock);
  switch(ret)
  {
    case detail::WaitResult::HasData:     return true;
    case detail::WaitResult::Timeout:     return false;
    case detail::WaitResult::Interrupted: return false;
  }
}

}
