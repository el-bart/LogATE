#pragma once
#include "But/Mpl/FreeOperators.hpp"
#include <cinttypes>

namespace LogATE
{

struct SequenceNumber final
{
  static SequenceNumber next();

  uint64_t value_{0};
};

BUT_MPL_FREE_OPERATORS_COMPARE(SequenceNumber, .value_);

}
