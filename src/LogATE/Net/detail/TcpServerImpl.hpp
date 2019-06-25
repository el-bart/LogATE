#pragma once
#include <vector>
#include <But/Optional.hpp>
#include <But/System/Descriptor.hpp>
#include "LogATE/Net/Port.hpp"
#include "LogATE/Net/Exception.hpp"
#include "LogATE/Net/Socket.hpp"
#include "LogATE/Net/SocketDescriptorPair.hpp"

namespace LogATE::Net::detail
{

class TcpServerImpl final
{
public:
  BUT_DEFINE_EXCEPTION(Error, Exception, "socket error");

  explicit TcpServerImpl(Port port);
  ~TcpServerImpl() = default;

  /** @brief interrupts pending accept() request. can be called from a different thread.
   */
  void interrupt();

  But::Optional<Socket> accept();

private:
  bool waitForConnection();

  But::System::Descriptor sock;
  SocketDescriptorPair sdp;
};

}
