#include "CursATE/Screen/detail/id2key.hpp"
#include "LogATE/Utils/zeroPaddedString.hpp"
#include <But/assert.hpp>
#include <boost/lexical_cast.hpp>

namespace CursATE::Screen::detail
{

LogATE::Log::Key id2key(Curses::DataSource::Id const& id)
{
  const auto splitPos = id.value_.rfind('/');
  BUT_ASSERT( splitPos != std::string::npos );
  const auto splitPoint = id.value_.begin() + splitPos;
  auto str = std::string{ id.value_.begin(), splitPoint };
  auto numStr = std::string_view{ id.value_.data() + splitPos + 1 };
  auto num = boost::lexical_cast<uint64_t>(numStr);
  return LogATE::Log::Key{ std::move(str), LogATE::SequenceNumber{num} };
}


Curses::DataSource::Id key2id(LogATE::Log::Key const& key)
{
  return Curses::DataSource::Id{ key.str() + "/" + LogATE::Utils::zeroPaddedString( key.sequenceNumber().value_ ) };
}

}
