#include "CursATE/Main.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"

namespace CursATE
{

Main::Main(const LogATE::Net::Port port):
  root_{ But::makeSharedNN<LogATE::Tree::Filter::AcceptAll>(LogATE::Tree::Node::Name{"root"}) },
  server_{port}
{
}

Main::~Main()
{
  quit_ = true;
  server_.interrupt();
}

void Main::run()
{
  // TODO
}

void Main::dataPumpLoop()
{
  while(not quit_)
    try
    {
      auto opt = server_.readNextLog();
      if(not opt)
        continue;
      root_->insert( std::move(*opt) );
    }
    catch(...)
    {
      // log down! MEDIC!
    }
}

}
