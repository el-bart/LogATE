#include "CursATE/Screen/LogDisplay/OrderedPrettyPrint.hpp"

namespace CursATE::Screen::LogDisplay
{

std::string OrderedPrettyPrint::operator()(LogATE::Log const& in) const
{
  (void)in;
  return "foo/bar"; // TODO...
}

}
