#include "LogATE/Log.hpp"

namespace LogATE::Printers
{

std::string jsonLine(LogATE::Log const& in)
{
  return in.log_->dump();
}

}
