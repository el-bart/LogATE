#include "LogATE/SequenceNumber.hpp"
#include <iostream>
#include <atomic>

namespace LogATE
{

namespace
{
auto nextFreeSN()
{
  static std::atomic<uint64_t> nextFree{0};
  return nextFree++;
}
}

SequenceNumber SequenceNumber::next()
{
  return SequenceNumber{ nextFreeSN() };
}

std::ostream& operator<<(std::ostream& os, const SequenceNumber sn)
{
  return os << "S/N=" << sn.value_;
}

}
