#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <But/assert.hpp>
#include "LogATE/Net/Socket.hpp"
#include "LogATE/Net/detail/helpers.hpp"

namespace LogATE::Net
{

Socket::Socket(But::System::Descriptor clientSocket) : sock_{std::move(clientSocket)}
{
  if( not sock_.opened() )
    BUT_THROW(Exception, "descriptor used for socket creation cannot be closed/invalid");
  detail::makeNonBlocking(sock_.get());
  detail::makeNonBlocking(sdp_.first.get());
  detail::makeNonBlocking(sdp_.second.get());
}


void Socket::interrupt()
{
  const auto wrapper = [fd = sdp_.first.get()] { return ::write(fd, "x", 1); };
  if( detail::sysCallWrapper(wrapper) == -1 )
    BUT_THROW(Error, "interruption failed - write(): " << strerror(errno));
}


std::string_view Socket::read(std::string& buffer)
{
  if( buffer.empty() )
    return {};
  const auto bytes = buffer.size();
  auto ready = size_t{0};
  while(ready != bytes)
  {
    BUT_ASSERT(sock_.opened());
    if( not waitForData(ReadyFor::Read) )
    {
      // interruption requested
      return std::string_view{ buffer.data(), static_cast<size_t>(ready) };
    }
    const auto wrapper = [fd=sock_.get(), ptr=buffer.data(), ready, bytes] { return ::read(fd, ptr + ready, bytes - ready); };
    auto ret = detail::sysCallWrapper(wrapper);
    if(ret == -1)
      BUT_THROW(Error, "read() failed: " << strerror(errno));
    if(ret == 0)
    {
      // remote socket closed
      return std::string_view{ buffer.data(), static_cast<size_t>(ready) };
    }
    BUT_ASSERT(ret > 0);
    ready += ret;
  }
  BUT_ASSERT(ready == bytes);
  return std::string_view{ buffer.data(), bytes };
}


std::string_view Socket::readSome(std::string& buffer)
{
  return readSomeImpl(buffer);
}


std::string_view Socket::readSome(std::string& buffer, const std::chrono::milliseconds timeout)
{
  return readSomeImpl(buffer, timeout);
}


size_t Socket::write(std::string const& data)
{
  auto bytesLeft = data.size();
  auto written = size_t{0};
  while(bytesLeft)
  {
    BUT_ASSERT(sock_.opened());
    if( not waitForData(ReadyFor::Write) )
      return written;
    const auto wrapper = [fd = sock_.get(), ptr = data.data(), written, bytesLeft]() { return ::write(fd, ptr + written, bytesLeft); };
    auto ret = detail::sysCallWrapper(wrapper);
    if(ret == -1)
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
  return detail::waitForData(sdp_.second, op, sock_);
}


bool Socket::waitForData(ReadyFor op, const std::chrono::milliseconds timeout)
{
  return detail::waitForData(sdp_.second, op, sock_, timeout);
}


template<typename ...Args>
std::string_view Socket::readSomeImpl(std::string& buffer, Args const&... args)
{
  if( buffer.empty() )
    return {};

  BUT_ASSERT(sock_.opened());
  if( not waitForData(ReadyFor::Read, args...) )
  {
    // interruption requested
    return {};
  }
  const auto wrapper = [fd=sock_.get(), ptr=buffer.data(), bytes=buffer.size()] { return ::read(fd, ptr, bytes); };
  auto ret = detail::sysCallWrapper(wrapper);
  if(ret == -1)
    BUT_THROW(Error, "read() failed: " << strerror(errno));
  if(ret == 0)
  {
    // remote socket closed
    return {};
  }
  BUT_ASSERT(ret > 0);
  return std::string_view{ buffer.data(), static_cast<uint64_t>(ret) };
}

}
