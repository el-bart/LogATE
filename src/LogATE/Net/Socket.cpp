#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <iostream>
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


std::pair<Socket::Reason, std::string_view> Socket::read(std::string& buffer)
{
  if( buffer.empty() )
    return std::make_pair( Reason::NoData, std::string_view{} );
  const auto bytes = buffer.size();
  auto ready = size_t{0};
  while(ready != bytes)
  {
    BUT_ASSERT(sock_.opened());
    switch( waitForData(ReadyFor::Read) )
    {
      case detail::WaitResult::Timeout:
           return std::make_pair( Reason::Timeout, std::string_view{ buffer.data(), static_cast<size_t>(ready) } );
      case detail::WaitResult::Interrupted:
           return std::make_pair( Reason::Interrupted, std::string_view{ buffer.data(), static_cast<size_t>(ready) } );
      case detail::WaitResult::HasData:
           break;
    }
    const auto wrapper = [fd=sock_.get(), ptr=buffer.data(), ready, bytes] { return ::read(fd, ptr + ready, bytes - ready); };
    auto ret = detail::sysCallWrapper(wrapper);
    if(ret == -1)
      BUT_THROW(Error, "read() failed: " << strerror(errno));
    if(ret == 0)
      return std::make_pair( Reason::ClosedByRemoteEnd, std::string_view{ buffer.data(), static_cast<size_t>(ready) } );
    BUT_ASSERT(ret > 0);
    ready += ret;
  }
  BUT_ASSERT(ready == bytes);
  return std::make_pair( Reason::Ok, std::string_view{ buffer.data(), bytes } );
}


std::pair<Socket::Reason, std::string_view> Socket::readSome(std::string& buffer)
{
  return readSomeImpl(buffer);
}


std::pair<Socket::Reason, std::string_view> Socket::readSome(std::string& buffer, const std::chrono::milliseconds timeout)
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
    switch( waitForData(ReadyFor::Write) )
    {
      case detail::WaitResult::Interrupted:
      case detail::WaitResult::Timeout: return written;
      case detail::WaitResult::HasData: break;
    }
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


detail::WaitResult Socket::waitForData(const ReadyFor op)
{
  return detail::waitForData(sdp_.second, op, sock_);
}


detail::WaitResult Socket::waitForData(ReadyFor op, const std::chrono::milliseconds timeout)
{
  return detail::waitForData(sdp_.second, op, sock_, timeout);
}


template<typename ...Args>
std::pair<Socket::Reason, std::string_view> Socket::readSomeImpl(std::string& buffer, Args const&... args)
{
  if( buffer.empty() )
    return std::make_pair(Reason::NoData, std::string_view{});

  BUT_ASSERT(sock_.opened());
  switch( waitForData(ReadyFor::Read, args...) )
  {
    case detail::WaitResult::Interrupted: return std::make_pair(Reason::Interrupted, std::string_view{});
    case detail::WaitResult::Timeout:     return std::make_pair(Reason::Timeout,     std::string_view{});
    case detail::WaitResult::HasData:     break;
  }

  const auto wrapper = [fd=sock_.get(), ptr=buffer.data(), bytes=buffer.size()] { return ::read(fd, ptr, bytes); };
  auto ret = detail::sysCallWrapper(wrapper);
  if(ret == -1)
    BUT_THROW(Error, "read() failed: " << strerror(errno));
  if(ret == 0)
    return std::make_pair(Reason::ClosedByRemoteEnd, std::string_view{});
  BUT_ASSERT(ret > 0);
  return std::make_pair(Reason::Ok, std::string_view{ buffer.data(), static_cast<uint64_t>(ret) });
}


std::ostream& operator<<(std::ostream& os, const Socket::Reason reason)
{
  switch(reason)
  {
    case Socket::Reason::Ok: return os << "Reason::Ok";
    case Socket::Reason::NoData: return os << "Reason::NoData";
    case Socket::Reason::Timeout: return os << "Reason::Timeout";
    case Socket::Reason::Interrupted: return os << "Reason::Interrupted";
    case Socket::Reason::ClosedByRemoteEnd: return os << "Reason::ClosedByRemoteEnd";
  }
  BUT_ASSERT(!"unknown Socket::Reason");
  throw std::logic_error{"unknown Socket::Reason: " + std::to_string(static_cast<unsigned>(reason))};
}

}
