#include "LogATE/Tree/Logs.hpp"

namespace LogATE::Tree
{

namespace
{
auto orderBySerialNumber(Log const& lhs, Log const& rhs)
{
  return lhs.sn_ < rhs.sn_;
}
}

void Logs::insert(Log log)
{
  BUT_ASSERT( std::is_sorted
  (void)log;
                
}

size_t Logs::pruneUpTo(SequenceNumber firstToKeep)
{
  (void)firstToKeep;
                
  return 24;
}

std::vector<Log> Logs::range(SequenceNumber begin, SequenceNumber end) const
{
  (void)begin;
  (void)end;
  return {};
}

std::vector<Log> Logs::from(SequenceNumber first, size_t count) const
{
  (void)first;
  (void)count;
  return {};
}

std::vector<Log> Logs::to(SequenceNumber last, size_t count) const
{
  (void)last;
  (void)count;
  return {};
}

}
