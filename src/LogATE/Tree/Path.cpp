#include "LogATE/Tree/Path.hpp"
#include <boost/tokenizer.hpp>
#include <regex>
#include <sstream>

namespace LogATE::Tree
{

Path::Entry::Entry(std::string str)
{
  if( str.empty() )
    BUT_THROW(EmptyNode, "node name cannot be empty");

  if( str.find('.') != std::string::npos )
    BUT_THROW(InvalidNode, str);

  // handle regular node
  {
    static const std::regex re{R"(([^\[\]]+))"};
    if( std::regex_match(str, re) )
    {
      name_ = std::move(str);
      return;
    }
  }

  // handle array
  {
    static const auto reStr = std::string{R"(([^\[\]]+)\[\s*([0-9]*)\s*\])"};
    static const std::regex re{reStr};
    std::smatch m;
    if( not std::regex_match(str, m, re) )
      BUT_THROW(InvalidArray, "array node definition is invalid: failed to match regex: " << reStr);
    isArray_ = true;
    BUT_ASSERT( m.size() == 1 + 2 );
    name_ = m[1];
    auto nStr = std::string{m[2]};
    if( nStr.empty() )
      return;
    hasIndex_ = true;
    index_ = stoull(nStr);
  }
}


std::string Path::Entry::str() const
{
  if( not isArray() )
    return name_;
  std::stringstream ss;
  ss << name_ << '[';
  if( hasIndex() )
    ss << index();
  ss << ']';
  return ss.str();
}


Path Path::parse(std::string const& str)
{
  if( str.empty() )
    return Path{};
  if( str == "." )
    return Path{{}, true};

  if( str.find("..") != std::string::npos )
    BUT_THROW(EmptyNodeInPath, str);
  if( str.back() == '.' )
    BUT_THROW(EmptyNodeInPath, str);

  const auto isAbsolute = ( str[0] == '.' );
  Path::Data data;

  boost::char_separator<char> sep(".");
  boost::tokenizer<boost::char_separator<char>> tokens{str, sep};
  for(auto it=tokens.begin(); it!=tokens.end(); ++it)
    data.push_back( Entry{*it} );
  return Path{data, isAbsolute};
}


std::string Path::str() const
{
  if( empty() )
    return "";
  std::stringstream ss;
  auto separator = "";
  for(auto& e: value_)
  {
    ss << separator << e.str();
    separator = ".";
  }
  return ss.str();
}

}
