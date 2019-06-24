#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
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

  LogATE::Json::Selector s;
  std::string buf;
  buf.resize(10*1024*1024);
  do
  {
    const auto size = read(0, buf.data(), buf.size());
    for(auto i=0; i<size; ++i)
    {
      s.update(buf[i]);
      if( s.jsonComplete() )
      {
        std::cout << s.str() << std::endl;
        s.reset();
      }
    }
  }
  while(not g_end); // TODO: continue if size >0
}
