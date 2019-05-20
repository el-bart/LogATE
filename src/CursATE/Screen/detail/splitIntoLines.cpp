#include "CursATE/Screen/detail/splitIntoLines.hpp"
#include <sstream>


namespace CursATE::Screen::detail
{

std::vector<std::string> splitIntoLines(std::string const& in, const size_t availableColumns)
{
  const auto columns = std::max<size_t>(80u/2u, availableColumns);  // if needed, horizontal scroll will do the trick here...
  (void) columns;           
  std::vector<std::string> out;
  auto it = begin(in);
  while( it != end(in) )
  {
    break;
    // TODO...
  }
  return out;
}

}
