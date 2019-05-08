#include "LogATE/Tree/Path.hpp"
#include <boost/tokenizer.hpp>

namespace LogATE::Tree
{

Path Path::parse(std::string const& str)
{
  if( str.empty() )
    return Path{};
  if( str == "." )
    return Path{{"."}};
  if( str.find("..") != std::string::npos )
    BUT_THROW(EmptyNodeInPath, str);
  if( str.back() == '.' )
    BUT_THROW(EmptyNodeInPath, str);

  Path::Data d;
  if( str[0]== '.' )
    d.push_back(".");

  boost::char_separator<char> sep(".");
  boost::tokenizer<boost::char_separator<char>> tokens{str, sep};
  for(auto it=tokens.begin(); it!=tokens.end(); ++it)
    d.push_back(*it);
  return Path{d};
}


std::string Path::str() const
{
  if( empty() )
    return "";
  std::stringstream ss;
  auto separator = "";
  for(auto& e: value_)
  {
    ss << separator << e;
    if( e != "." )
      separator = ".";
  }
  return ss.str();
}

}
