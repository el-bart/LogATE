#include "CursATE/Main.hpp"
#include <iostream>

using LogATE::Net::Port;


int main()
{
  try
  {
    CursATE::Main main{Port{4242}};
    main.run();
  }
  catch(std::exception const& ex)
  {
    std::cerr << "fatal exception: " << ex.what() << std::endl;
    return 3;
  }
}
