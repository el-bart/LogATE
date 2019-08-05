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
    return addToCache( std::move(key) );
  if( cacheLb->key_ == key )
    return cacheLb->index_;

  const auto cacheUb = std::upper_bound( cacheLb, cache_.end(), key, OrderByKey{} );
  if( cacheUb == cache_.end() )
    return addToCache( cacheLb->key_, cacheLb->index_, std::move(key) );

  return addToCache( cacheLb->key_, cacheLb->index_, cacheUb->key_, cacheUb->index_, std::move(key) );
}


void LogIdIndexCache::invalidateCacheOnChange() const
{
  if( not cache_.empty() )
    BUT_ASSERT(firstKey_);
  const auto ll = node_.lock()->logs().withLock();
  if( ll->empty() )
    return;
  if(firstKey_)
    if( *firstKey_ == ll->first().key() )
      return;
  cache_.clear();
  firstKey_ = ll->first().key();
}


size_t LogIdIndexCache::addToCache(LogATE::Log::Key&& key) const
{
  const auto ll = node_.lock()->logs().withLock();
  return addToCache( ll->begin(), 0, ll->end(), ll->size(), std::move(key) );
}


size_t LogIdIndexCache::addToCache(LogATE::Log::Key const& lb, size_t lbPos, LogATE::Log::Key&& key) const
{
  const auto ll = node_.lock()->logs().withLock();
  return addToCache( ll->find(lb), lbPos, ll->end(), ll->size(), std::move(key) );
}


size_t LogIdIndexCache::addToCache(LogATE::Log::Key const& lb, size_t lbPos,
                                   LogATE::Log::Key const& ub, size_t ubPos,
                                   LogATE::Log::Key&& key) const
{
  const auto ll = node_.lock()->logs().withLock();
  return addToCache( ll->find(lb), lbPos, ll->find(ub), ubPos, std::move(key) );
}


size_t LogIdIndexCache::addToCache(LogATE::Tree::Logs::const_iterator lb, size_t lbPos,
                                   LogATE::Tree::Logs::const_iterator ub, size_t ubPos,
                                   LogATE::Log::Key&& key) const
{
  if( lb == ub )
    return 0;

  BUT_ASSERT( lbPos < ubPos );
  while( lbPos < ubPos )
  {
    if( lb->key() == key )
    {
      const auto it = std::lower_bound( cache_.begin(), cache_.end(), key, OrderByKey{} );
      cache_.insert( it, Entry{ std::move(key), lbPos } );
      return lbPos;
    }
    ++lb;
    ++lbPos;

    --ub;
    --ubPos;
    if( ub->key() == key )
    {
      const auto it = std::lower_bound( cache_.begin(), cache_.end(), key, OrderByKey{} );
      cache_.insert( it, Entry{ std::move(key), ubPos } );
      return ubPos;
    }
  }
  return 0;
}

}
