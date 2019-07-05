#pragma once
#include "LogATE/Log.hpp"
#include "But/Threading/BasicLockable.hpp"
#include "But/Threading/LockProxyProvider.hpp"
#include "But/assert.hpp"
#include <string>
#include <deque>
#include <vector>
#include <mutex>

namespace LogATE::Tree
{

class Logs: public But::Threading::BasicLockable<Logs>,
            public But::Threading::LockProxyProvider<Logs>
{
public:
  auto size() const { BUT_ASSERT( locked() ); return logs_.size(); }
  auto empty() const { BUT_ASSERT( locked() ); return size() == 0u; }

  Log const& first() const { BUT_ASSERT( locked() ); BUT_ASSERT( not empty() ); return logs_.front(); }
  Log const& last()  const { BUT_ASSERT( locked() ); BUT_ASSERT( not empty() ); return logs_.back(); }

  auto begin()       { BUT_ASSERT( locked() ); return logs_.begin(); }
  auto end()         { BUT_ASSERT( locked() ); return logs_.end(); }
  auto begin() const { BUT_ASSERT( locked() ); return logs_.begin(); }
  auto end()   const { BUT_ASSERT( locked() ); return logs_.end(); }

  auto rbegin()       { BUT_ASSERT( locked() ); return logs_.rbegin(); }
  auto rend()         { BUT_ASSERT( locked() ); return logs_.rend(); }
  auto rbegin() const { BUT_ASSERT( locked() ); return logs_.rbegin(); }
  auto rend()   const { BUT_ASSERT( locked() ); return logs_.rend(); }

  std::deque<Log>::iterator find(Log::Key const& key);
  std::deque<Log>::const_iterator find(Log::Key const& key) const;

  void insert(Log log);

  size_t pruneUpTo(Log::Key const& firstToKeep);

  std::vector<Log> range(Log::Key const& begin, Log::Key const& end) const;
  std::vector<Log> from(Log::Key const& first, size_t count) const;
  std::vector<Log> to(Log::Key const& last, size_t count) const;

private:
  std::deque<Log> logs_;
};

}
