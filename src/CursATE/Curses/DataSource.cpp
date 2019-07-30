#include "CursATE/Curses/DataSource.hpp"
#include <sstream>
#include <iomanip>

namespace CursATE::Curses
{

namespace
{
auto toZeroPaddedString(uint64_t value)
{
  const auto maxDigits = std::numeric_limits<decltype(value)>::digits10 + 1;
  std::stringstream ss;
  ss << std::setw(maxDigits) << std::setfill('0') << value;
  return ss.str();
}
}

DataSource::Id::Id(uint64_t value):
  value_{ toZeroPaddedString(value) }
{ }

}
