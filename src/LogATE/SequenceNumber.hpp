#pragma once
#include <cinttypes>

namespace LogATE
{

struct SequenceNumber final
{
  SequenceNumber();

  uint64_t value_;
};

}
