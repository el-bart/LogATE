#include "CursATE/Screen/detail/id2key.hpp"
#include "LogATE/Utils/string2key.hpp"

namespace CursATE::Screen::detail
{

LogATE::Log::Key id2key(Curses::DataSource::Id const& id)
{
  return LogATE::Utils::string2key(id.value_);
}


Curses::DataSource::Id key2id(LogATE::Log::Key const& key)
{
  return Curses::DataSource::Id{ key.str() + "/" + LogATE::Utils::zeroPaddedString( key.sequenceNumber().value_ ) };
}

}
