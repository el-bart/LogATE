#pragma once
#include "But/Mpl/FreeOperators.hpp"
#include <cinttypes>
#include <iosfwd>

namespace LogATE
{

struct SequenceNumber final
{
  static SequenceNumber next();

  SequenceNumber() = delete;
  explicit SequenceNumber(uint64_t value): value_{value} { }

  uint64_t value_;
};

BUT_MPL_FREE_OPERATORS_COMPARE(SequenceNumber, .value_);

std::ostream& operator<<(std::ostream& os, SequenceNumber sn);

}
