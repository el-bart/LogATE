#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "SocketDescriptorPair.hpp"

namespace LogATE::Net
{

SocketDescriptorPair::SocketDescriptorPair()
{
  int fds[2];
  if (socketpair(AF_LOCAL, SOCK_STREAM, PF_UNSPEC, fds) != 0)
    BUT_THROW(Exception, "socketpair() failed: " << strerror(errno));
  first.reset(fds[0]);
  second.reset(fds[1]);
}

}
