#pragma once
#include "LogATE/Log.hpp"
#include "LogATE/Tree/Node.hpp"
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
  struct Entry
  {
    LogATE::Log::Key key_;
    size_t index_;
  };

  const LogATE::Tree::NodeWeakPtr node_;
  mutable std::mutex mutex_;
  mutable std::vector<Entry> cache_;
};

}
