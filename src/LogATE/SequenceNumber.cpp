#include "LogATE/SequenceNumber.hpp"
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

}
