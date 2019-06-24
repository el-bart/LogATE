#include <string>
#include <iostream>
#include "LogATE/Json/Selector.hpp"

int main()
{
  LogATE::Json::Selector s;
  std::string line;
  while( getline(std::cin, line) )
    for(auto c: line)
    {
      s.update(c);
      if( s.jsonComplete() )
      {
        std::cout << s.str() << std::endl;
        s.reset();
      }
    }
}
