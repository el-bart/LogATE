#include "CursATE/Main.hpp"
#include "CursATE/extractConfig.hpp"
#include <iostream>

using LogATE::Net::Port;


int main(int argc, char** argv)
{
  try
  {
    CursATE::Main main{ CursATE::extractConfig(argc, argv) };
    main.run();
  }
  catch(std::exception const& ex)
  {
    std::cerr << "fatal exception: " << ex.what() << std::endl;
    return 3;
  }
}
