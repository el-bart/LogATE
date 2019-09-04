#pragma once
#include "LogATE/Log.hpp"
#include <vector>

namespace LogATE::Net
{

struct Server
{
  virtual ~Server() = default;

  virtual std::vector<AnnotatedLog> readNextLogs() = 0;
  virtual void interrupt() = 0;
  virtual size_t errors() const = 0;
};

}
