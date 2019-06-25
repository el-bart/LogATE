#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <But/assert.hpp>
#include "LogATE/Net/Socket.hpp"
#include "LogATE/Net/detail/helpers.hpp"

namespace LogATE::Net
{

Socket::Socket(But::System::Descriptor clientSocket) : sock{std::move(clientSocket)}
{
  if( not sock.opened() )
    BUT_THROW(Exception, "descriptor used for socket creation cannot be closed/invalid");
  detail::makeNonBlocking(sock.get());
  detail::makeNonBlocking(sdp.first.get());
  detail::makeNonBlocking(sdp.second.get());
}

void Socket::interrupt()
{
  const auto wrapper = [fd = sdp.first.get()] { return ::write(fd, "x", 1); };
  if( detail::sysCallWrapper(wrapper) == -1 )
    BUT_THROW(Error, "interruption failed - write(): " << strerror(errno));
}

std::string Socket::read(const size_t bytes)
{
  std::string out;
  out.resize(bytes);
  auto ready = size_t{0};
  while(ready != bytes)
  {
    BUT_ASSERT(sock.opened());
    if( not waitForData(ReadyFor::Read) )
    {
      // interruption requested
      out.resize(ready);
      return out;
    }
    const auto wrapper = [fd = sock.get(), ptr = &out[0], ready, bytes] { return ::read(fd, ptr + ready, bytes - ready); };
    auto ret = detail::sysCallWrapper(wrapper);
    if(ret == -1)
      BUT_THROW(Error, "read() failed: " << strerror(errno));
    if(ret == 0)
    {
      // remote socket closed
      out.resize(ready);
      return out;
    }
    BUT_ASSERT(ret > 0);
    ready += ret;
  }
  BUT_ASSERT(out.size() == bytes);
  return out;
}

size_t Socket::write(std::string const& data)
{
  auto bytesLeft = data.size();
  auto written = size_t{0};
  while (bytesLeft)
  {
    BUT_ASSERT(sock.opened());
    if (not waitForData(ReadyFor::Write))
      return written;
    const auto wrapper = [fd = sock.get(), ptr = data.data(), written, bytesLeft]()
    {
      return ::write(fd, ptr + written, bytesLeft);
    };
    auto ret = detail::sysCallWrapper(wrapper);
    if (ret == -1)
      BUT_THROW(Error, "write() failed: " << strerror(errno));
    BUT_ASSERT(ret >= 0);
    bytesLeft -= ret;
    written += ret;
  }
  BUT_ASSERT(bytesLeft == 0u);
  BUT_ASSERT(written == data.size());
  return data.size();
}

bool Socket::waitForData(const ReadyFor op)
{
  return detail::waitForData(sdp.second, op, sock);
}

}
