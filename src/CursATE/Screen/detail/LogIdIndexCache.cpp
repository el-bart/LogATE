#include "CursATE/Screen/detail/LogIdIndexCache.hpp"
#include <vector>
#include <mutex>
#include <But/assert.hpp>

namespace CursATE::Screen::detail
{

size_t LogIdIndexCache::index(LogATE::Log::Key key) const
{
  const std::lock_guard<std::mutex> lock{mutex_};
  const auto node = node_.lock();
  if(not node)
    return 0;
  invalidateCacheOnChange();

  BUT_ASSERT( std::is_sorted( cache_.begin(), cache_.end(), OrderByKey{} ) );
  const auto cacheLb = std::lower_bound( cache_.begin(), cache_.end(), key, OrderByKey{} );
  if( cacheLb == cache_.end() )
    return addToCacheAtTheEnd( std::move(key) );
  if( cacheLb->key_ == key )
    return cacheLb->index_;
  BUT_ASSERT( key < cacheLb->key_ );
  return addToCacheLeftOf( cacheLb, std::move(key) );
}


void LogIdIndexCache::invalidateCacheOnChange() const
{
  if( not cache_.empty() )
    BUT_ASSERT(firstKey_);
  const auto ll = node_.lock()->logs().withLock();
  if( ll->empty() )
  {
    reset();
    return;
  }
  if(firstKey_)
    if( *firstKey_ == ll->first().key() )
      return;
  // TODO: this does not need to be a full reset - it is enough to decrease index_ of all entries by a removed elements.
  reset();
  firstKey_ = ll->first().key();
}


void LogIdIndexCache::reset() const
{
  cache_.clear();
  firstKey_.reset();
}


size_t LogIdIndexCache::addToCacheAtTheEnd(LogATE::Log::Key&& key) const
{
  const auto ll = node_.lock()->logs().withLock();
  const auto it = ll->find(key);
  if( it == ll->end() )
    return 0;

  if( cache_.empty() )
  {
    const auto pos = static_cast<size_t>( std::distance( ll->begin(), it ) );
    firstKey_ = ll->first().key();
    cache_.push_back( Entry{ std::move(key), pos } );
    BUT_ASSERT( std::is_sorted( cache_.begin(), cache_.end(), OrderByKey{} ) );
    return pos;
  }

  BUT_ASSERT( cache_.rbegin()->key_ < key );
  const auto& last = cache_.back();
  const auto lastIt = ll->find(last.key_);
  const auto pos = static_cast<size_t>( std::distance( lastIt, it ) ) + last.index_;
  cache_.push_back( Entry{ std::move(key), pos } );
  BUT_ASSERT( std::is_sorted( cache_.begin(), cache_.end(), OrderByKey{} ) );
  return pos;
}


size_t LogIdIndexCache::addToCacheLeftOf(const std::vector<Entry>::const_iterator it, LogATE::Log::Key&& key) const
{
  BUT_ASSERT( it != cache_.end() );
  const auto ll = node_.lock()->logs().withLock();
  const auto keyIt = ll->find(key);
  const auto nextKnownIt = ll->find(it->key_);
  const auto pos = it->index_ - static_cast<size_t>( std::distance( keyIt, nextKnownIt ) );
  cache_.insert( it, Entry{ std::move(key), pos } );
  BUT_ASSERT( std::is_sorted( cache_.begin(), cache_.end(), OrderByKey{} ) );
  return pos;
}

}
