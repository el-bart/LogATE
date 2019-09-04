#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <cstring>
#include <atomic>
#include <iostream>
#include "LogATE/Json/Selector.hpp"

std::atomic<bool> g_end{false};

void handleSigPipe(const int s)
{
  std::cerr << "\n\n" << "SIG = " << s << " stopping..." << std::endl;
  g_end = true;
}


int main()
{
  {
    const auto fd = 0;
    const auto flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  }

  for(auto sig: {SIGPIPE, SIGTERM, SIGINT, SIGQUIT, SIGHUP})
    if( signal(sig, handleSigPipe) == SIG_ERR )
      throw std::runtime_error{"oops..."};

#if 1
  LogATE::Json::Selector s;
#else
  LogATE::Json::NewLineSplit s;
#endif

  std::string buf;
  buf.resize(10*1024*1024);
  while(not g_end)
  {
    const auto size = read(0, buf.data(), buf.size());
    if(size == -1 && errno != EAGAIN &&  errno != EWOULDBLOCK && errno != EINTR )
    {
      std::cerr << "ERROR from read(): " << strerror(errno) << std::endl;
      return 3;
    }
    if(size == 0)
      return 0;
    for(auto i=0; i<size; ++i)
    {
      s.update(buf[i]);
      if( s.jsonComplete() )
      {
        const auto str = std::string{ s.str() } + "\n";
        const auto out = write(1, str.data(), str.size());
        if( out == -1 )
        {
          std::cerr << "ERROR from write(): " << strerror(errno) << std::endl;
          return 4;
        }
        if( static_cast<uint64_t>(out) != str.size() )
        {
          std::cerr << "ERROR from write(): " << strerror(errno) << std::endl;
          return 0;
        }
        s.reset();
      }
    }
  }
}
