#pragma once
#include "LogATE/Log.hpp"
#include <But/Optional.hpp>

namespace LogATE::Net
{

struct Server
{
  virtual ~Server() = default;

  virtual But::Optional<Log> readNextLog() = 0;
  virtual void interrupt() = 0;
};

}
