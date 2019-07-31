#pragma once
#include "LogATE/Log.hpp"
#include "But/Threading/BasicLockable.hpp"
#include "But/Threading/LockProxyProvider.hpp"
#include "But/assert.hpp"
#include <string>
#include <set>
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

  auto begin()       { BUT_ASSERT( locked() ); return logs_.begin(); }
  auto end()         { BUT_ASSERT( locked() ); return logs_.end(); }
  auto begin() const { BUT_ASSERT( locked() ); return logs_.begin(); }
  auto end()   const { BUT_ASSERT( locked() ); return logs_.end(); }

  auto rbegin()       { BUT_ASSERT( locked() ); return logs_.rbegin(); }
  auto rend()         { BUT_ASSERT( locked() ); return logs_.rend(); }
  auto rbegin() const { BUT_ASSERT( locked() ); return logs_.rbegin(); }
  auto rend()   const { BUT_ASSERT( locked() ); return logs_.rend(); }

  std::set<Log>::iterator find(Log::Key const& key);
  std::set<Log>::const_iterator find(Log::Key const& key) const;

  template<typename Key>
  auto lower_bound(Key const& key)       { BUT_ASSERT( locked() ); return logs_.lower_bound(key); }
  template<typename Key>
  auto lower_bound(Key const& key) const { BUT_ASSERT( locked() ); return logs_.lower_bound(key); }

  template<typename Key>
  auto upper_bound(Key const& key)       { BUT_ASSERT( locked() ); return logs_.upper_bound(key); }
  template<typename Key>
  auto upper_bound(Key const& key) const { BUT_ASSERT( locked() ); return logs_.upper_bound(key); }

  Log const& first() const { BUT_ASSERT( locked() ); BUT_ASSERT( not empty() ); return *begin(); }
  Log const& last()  const { BUT_ASSERT( locked() ); BUT_ASSERT( not empty() ); return *rbegin(); }

  void insert(Log log);

  size_t pruneUpTo(Log::Key const& firstToKeep);

  std::vector<Log> range(Log::Key const& begin, Log::Key const& end) const;
  std::vector<Log> from(Log::Key const& first, size_t count) const;
  std::vector<Log> to(Log::Key const& last, size_t count) const;

private:
  std::set<Log, OrderByKey> logs_;
};

}
