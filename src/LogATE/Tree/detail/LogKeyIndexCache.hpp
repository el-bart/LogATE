#pragma once
#include "LogATE/Log.hpp"
#include <But/assert.hpp>
#include <vector>
#include <set>

namespace LogATE::Tree::detail
{

class LogKeyIndexCache final
{
public:
  using Data = std::set<Log, OrderByKey>;

  explicit LogKeyIndexCache(Data const* data, const size_t insertEveryNth = 100'000, const size_t cacheIfDistanceAbove = 6'000):
    data_{data},
    insertEveryNth_{insertEveryNth},
    cacheIfDistanceAbove_{cacheIfDistanceAbove}
  {
    BUT_ASSERT(data_);
  }

  size_t index(LogATE::Log::Key key);
  void updateAfterPruneUpTo(LogATE::Log::Key const& key);
  void updateAfterInsertion(LogATE::Log::Key const& key);
  auto size() const { return cache_.size(); }

private:
  struct Entry
  {
    LogATE::Log::Key key_;
    size_t index_;
  };

  size_t addToCacheAtTheEnd(LogATE::Log::Key&& key);
  size_t addToCacheLeftOf(std::vector<Entry>::const_iterator it, LogATE::Log::Key&& key);
  size_t addToCacheBetween(std::vector<Entry>::const_iterator low, std::vector<Entry>::const_iterator high, LogATE::Log::Key&& key);

  size_t cacheIfNoEntryCloseEnough(std::vector<Entry>::const_iterator it, LogATE::Log::Key&& key, size_t pos);

  struct CacheOrderByKey final
  {
    auto operator()(LogKeyIndexCache::Entry const& lhs, LogKeyIndexCache::Entry const& rhs) const { return lhs.key_ < rhs.key_; }
    auto operator()(LogKeyIndexCache::Entry const& lhs, LogATE::Log::Key        const& rhs) const { return lhs.key_ < rhs;      }
    auto operator()(LogATE::Log::Key        const& lhs, LogKeyIndexCache::Entry const& rhs) const { return lhs      < rhs.key_; }
  };

  Data const* data_;
  size_t insertEveryNth_;
  size_t insertionCounter_{0};
  size_t cacheIfDistanceAbove_;
  std::vector<Entry> cache_;
};

}
