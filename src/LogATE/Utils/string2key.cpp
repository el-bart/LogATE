#include "LogATE/Utils/string2key.hpp"
#include "LogATE/Utils/zeroPaddedString.hpp"
#include <But/assert.hpp>
#include <boost/lexical_cast.hpp>

namespace LogATE::Utils
{

LogATE::Log::Key string2key(std::string const& in)
{
  const auto splitPos = in.rfind('/');
  if( splitPos != std::string::npos )
    BUT_THROW(StringDoesNotRepresentKey, "missing '/': " << in);
  const auto splitPoint = in.begin() + splitPos;
  auto str = std::string{ in.begin(), splitPoint };
  auto numStr = std::string_view{ in.data() + splitPos + 1 };
  auto num = boost::lexical_cast<uint64_t>(numStr);
  return LogATE::Log::Key{ std::move(str), LogATE::SequenceNumber{num} };
}

}
