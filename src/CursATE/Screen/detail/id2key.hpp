#include "LogATE/Log.hpp"
#include "CursATE/Curses/DataSource.hpp"

namespace CursATE::Screen::detail
{

LogATE::Log::Key id2key(Curses::DataSource::Id const& id);
Curses::DataSource::Id key2id(LogATE::Log::Key const& key);

}
