#include "CursATE/Screen/detail/regexEscape.hpp"
#include <regex>

namespace CursATE::Screen::detail
{

std::string regexEscape(std::string const& in)
{
  static const std::regex re{R"([.^$|()\\[*+?{}\]])"};
  return std::regex_replace(in, re, R"(\$&)");
}

}
