#pragma once
#include "LogATE/Log.hpp"
#include "LogATE/Tree/Node.hpp"
#include <But/Optional.hpp>
#include <vector>
#include <mutex>

namespace CursATE::Screen::detail
{

class LogIdIndexCache final
{
public:
  explicit LogIdIndexCache(LogATE::Tree::NodeWeakPtr node):
    node_{ std::move(node) }
  { }

  size_t index(LogATE::Log::Key key) const;
  auto size() const { std::lock_guard<std::mutex> lock{mutex_}; return cache_.size(); }

private:
  void invalidateCacheOnChange() const;
  size_t addToCache(LogATE::Log::Key&& key) const;
  size_t addToCache(LogATE::Log::Key const& lb, size_t lbPos,
                    LogATE::Log::Key&& key) const;
  size_t addToCache(LogATE::Log::Key const& lb, size_t lbPos,
                    LogATE::Log::Key const& ub, size_t ubPos,
                    LogATE::Log::Key&& key) const;
  size_t addToCache(LogATE::Tree::Logs::const_iterator lb, size_t lbPos,
                    LogATE::Tree::Logs::const_iterator ub, size_t ubPos,
                    LogATE::Log::Key&& key) const;

  struct Entry
  {
    LogATE::Log::Key key_;
    size_t index_;
  };

  struct OrderByKey final
  {
    auto operator()(LogIdIndexCache::Entry const& lhs, LogIdIndexCache::Entry const& rhs) const { return lhs.key_ < rhs.key_; }
    auto operator()(LogIdIndexCache::Entry const& lhs, LogATE::Log::Key       const& rhs) const { return lhs.key_ < rhs;      }
    auto operator()(LogATE::Log::Key       const& lhs, LogIdIndexCache::Entry const& rhs) const { return lhs      < rhs.key_; }
  };

  const LogATE::Tree::NodeWeakPtr node_;
  mutable std::mutex mutex_;
  mutable std::vector<Entry> cache_;
  mutable But::Optional<LogATE::Log::Key> firstKey_;
};

}
