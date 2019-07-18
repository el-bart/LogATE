#pragma once
#include "But/Mpl/FreeOperators.hpp"
#include <cinttypes>

namespace LogATE
{

struct SequenceNumber final
{
  static SequenceNumber next();
  static SequenceNumber lastIssued();

  SequenceNumber() = delete;
  explicit SequenceNumber(uint64_t value): value_{value} { }

  uint64_t value_;
};

BUT_MPL_FREE_OPERATORS_COMPARE(SequenceNumber, .value_)

}
