#include "CursATE/Curses/Field/detail/resizePadded.hpp"

namespace CursATE::Curses::Field::detail
{

std::string resizePadded(std::string const& in, const size_t maxSize, const size_t selectedElement)
{
  if( in.size() <= maxSize )
  {
    auto tmp = in;
    tmp.resize(maxSize, ' ');
    return tmp;
  }
  if( selectedElement >= in.size() )
    BUT_THROW(SelectionOutOfRange, "requested element " << selectedElement << " in a string of length " << in.size());

  const auto half = maxSize / 2;
  auto start = 0u;
  if( selectedElement > half )
    start = selectedElement - half;
  if( in.size() - start < maxSize )
    start = in.size() - maxSize;
  return in.substr(start, maxSize);
}

}
