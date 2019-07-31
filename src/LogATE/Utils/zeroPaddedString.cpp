#include "LogATE/Utils/zeroPaddedString.hpp"
#include <limits>
#include <iomanip>
#include <sstream>

namespace LogATE::Utils
{

std::string zeroPaddedString(const uint64_t value)
{
  std::stringstream ss;
  constexpr auto maxDigits = std::numeric_limits<decltype(value)>::digits10 + 1u;
  ss << std::setfill('0') << std::setw(maxDigits) << value;
  return ss.str();
}

}
