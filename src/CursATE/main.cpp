#include "CursATE/Main.hpp"
#include "CursATE/extractConfig.hpp"
#include <iostream>

using LogATE::Net::Port;


int main(int argc, char** argv)
{
  try
  {
    auto cfg = CursATE::extractConfig(argc, argv);
    if(not cfg)
      return 3;
    CursATE::Main main{ std::move(*cfg) };
    main.run();
  }
  catch(std::exception const& ex)
  {
    std::cerr << "fatal exception: " << ex.what() << std::endl;
    return 4;
  }
}
