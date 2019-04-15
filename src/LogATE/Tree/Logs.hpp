#pragma once
#include "LogATE/SequenceNumber.hpp"
#include "LogATE/Log.hpp"
#include "But/Threading/BasicLockable.hpp"
#include "But/Threading/LockProxyProvider.hpp"
#include "But/assert.hpp"
#include <string>
#include <mutex>

namespace LogATE::Tree
{

class Logs: public But::Threading::BasicLockable<Logs>,
            public But::Threading::LockProxyProvider<Logs>
{
public:
  auto size() const
  {
    BUT_ASSERT( locked() );
    return logs_.size();
  }
  auto empty() const
  {
    BUT_ASSERT( locked() );
    return size() == 0u;
  }

  Log const& first() const
  {
    BUT_ASSERT( locked() );
    BUT_ASSERT( not empty() );
    return logs_.front();
  }

  Log const& last() const
  {
    BUT_ASSERT( locked() );
    BUT_ASSERT( not empty() );
    return logs_.back();
  }

  auto begin()
  {
    BUT_ASSERT( locked() );
    return logs_.begin();
  }
  auto end()
  {
    BUT_ASSERT( locked() );
    return logs_.end();
  }
  auto begin() const
  {
    BUT_ASSERT( locked() );
    return logs_.begin();
  }
  auto end() const
  {
    BUT_ASSERT( locked() );
    return logs_.end();
  }

  void insert(Log log);

  size_t pruneUpTo(SequenceNumber firstToKeep);

  std::vector<Log> range(SequenceNumber begin, SequenceNumber end) const;
  std::vector<Log> from(SequenceNumber first, size_t count) const;
  std::vector<Log> to(SequenceNumber last, size_t count) const;

private:
  std::vector<Log> logs_;
};

}
