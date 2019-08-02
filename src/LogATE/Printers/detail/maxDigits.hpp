#include <cinttypes>
#include <string>
#include <cmath>
#include <But/assert.hpp>

namespace LogATE::Printers::detail
{

inline auto maxDigits(const uint64_t value)
{
  if(value == 0)
    return 1u;
  const auto vlog = std::log10(value);
  const auto clog = std::floor(vlog)+1;
  const auto digits = static_cast<unsigned>(clog);
  BUT_ASSERT( digits == std::to_string(value).size() );
  return digits;
}

}
