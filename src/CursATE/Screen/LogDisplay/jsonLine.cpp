#include "LogATE/Log.hpp"

namespace CursATE::Screen::LogDisplay
{

std::string jsonLine(LogATE::Log const& in)
{
  return in.log_->dump();
}

}
