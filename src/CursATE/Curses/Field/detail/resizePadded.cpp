#include "CursATE/Curses/Field/detail/resizePadded.hpp"
#include <But/assert.hpp>

namespace CursATE::Curses::Field::detail
{

VisibleSize resizePaddedVisibleSize(std::string const& in, size_t maxSize, size_t selectedElement)
{
  if( in.size() <= maxSize )
    return {0, selectedElement, in.size()};
  if( selectedElement > in.size() )
    BUT_THROW(SelectionOutOfRange, "requested element " << selectedElement << " in a string of length " << in.size());

  const auto half = maxSize / 2;
  auto start = 0u;
  if( selectedElement > half )
    start = selectedElement - half;
  if( in.size() - start < maxSize )
    start = in.size() - maxSize;
  const auto offset = selectedElement - start;
  BUT_ASSERT( offset <= maxSize && "offset it outside of display window" );
  return {start, offset, maxSize};
}

std::string resizePadded(std::string const& in, const size_t maxSize, const size_t selectedElement)
{
  if( in.size() <= maxSize )
  {
    auto tmp = in;
    tmp.resize(maxSize, ' ');
    return tmp;
  }
  const auto vs = resizePaddedVisibleSize(in, maxSize, selectedElement);
  return in.substr(vs.start_, vs.count_);
}

}
