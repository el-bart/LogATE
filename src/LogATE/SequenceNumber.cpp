#include "LogATE/SequenceNumber.hpp"
#include <atomic>

namespace LogATE
{

namespace
{
auto& counter()
{
  static std::atomic<uint64_t> nextFree{0};
  return nextFree;
}
auto nextFreeSN()
{
  return counter()++;
}
}

SequenceNumber SequenceNumber::next()
{
  return SequenceNumber{ nextFreeSN() };
}


SequenceNumber SequenceNumber::lastIssued()
{
  return SequenceNumber{ counter().load() };
}

}
