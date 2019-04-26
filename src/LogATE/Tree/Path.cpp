#include "LogATE/Tree/Path.hpp"
#include <boost/tokenizer.hpp>

namespace LogATE::Tree
{

Path Path::parse(std::string const& str)
{
  if( str.empty() || str == "." )
    return {{"."}};
  if( str.find("..") != std::string::npos )
    BUT_THROW(EmptyNodeInPath, str);
  if( str.back() == '.' )
    BUT_THROW(EmptyNodeInPath, str);

  Path p;
  if( str[0]== '.' )
    p.value_.push_back(".");

  boost::char_separator<char> sep(".");
  boost::tokenizer<boost::char_separator<char>> tokens{str, sep};
  for(auto it=tokens.begin(); it!=tokens.end(); ++it)
    p.value_.push_back(*it);
  return p;
}


std::string Path::str() const
{
  if( value_.empty() )
    return ".";
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
