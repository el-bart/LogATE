#include "LogATE/Tree/detail/LogKeyIndexCache.hpp"
#include <vector>
#include <mutex>
#include <But/assert.hpp>

namespace LogATE::Tree::detail
{

size_t LogKeyIndexCache::index(LogATE::Log::Key key)
{
  BUT_ASSERT( std::is_sorted( cache_.begin(), cache_.end(), CacheOrderByKey{} ) );
  const auto cacheLb = std::lower_bound( cache_.begin(), cache_.end(), key, CacheOrderByKey{} );
  if( cacheLb == cache_.end() )
    return addToCacheAtTheEnd( std::move(key) );
  if( cacheLb->key_ == key )
    return cacheLb->index_;
  BUT_ASSERT( key < cacheLb->key_ );
  return addToCacheLeftOf( cacheLb, std::move(key) );
}


void LogKeyIndexCache::updateAfterInsertion(LogATE::Log::Key const& key)
{
  BUT_ASSERT( std::is_sorted( cache_.begin(), cache_.end(), CacheOrderByKey{} ) );
  for(auto it = std::upper_bound( cache_.begin(), cache_.end(), key, CacheOrderByKey{} ); it != cache_.end(); ++it)
    ++it->index_;
}


void LogKeyIndexCache::updateAfterPruneUpTo(LogATE::Log::Key const& key)
{
  BUT_ASSERT( std::is_sorted( cache_.begin(), cache_.end(), CacheOrderByKey{} ) );
  const auto cacheLb = std::lower_bound( cache_.begin(), cache_.end(), key, CacheOrderByKey{} );
  if( cacheLb == cache_.end() )
  {
    cache_.clear();
    return;
  }
  cache_.erase( cache_.begin(), cacheLb );
  if( cache_.empty() )
    return;
  const auto it = data_->find( cache_.begin()->key_ );
  const auto dist = static_cast<size_t>( std::distance( data_->begin(), it ) );
  const auto diff = cache_.begin()->index_ - dist;
  for(auto& e: cache_)
    e.index_ -= diff;
}


size_t LogKeyIndexCache::addToCacheAtTheEnd(LogATE::Log::Key&& key)
{
  const auto it = data_->find(key);
  if( it == data_->end() )
    return 0;

  if( cache_.empty() )
  {
    const auto pos = static_cast<size_t>( std::distance( data_->begin(), it ) );
    cache_.push_back( Entry{ std::move(key), pos } );
    BUT_ASSERT( std::is_sorted( cache_.begin(), cache_.end(), CacheOrderByKey{} ) );
    return pos;
  }

  BUT_ASSERT( cache_.rbegin()->key_ < key );
  const auto& last = cache_.back();
  const auto lastIt = data_->find(last.key_);
  const auto pos = static_cast<size_t>( std::distance( lastIt, it ) ) + last.index_;
  cache_.push_back( Entry{ std::move(key), pos } );
  BUT_ASSERT( std::is_sorted( cache_.begin(), cache_.end(), CacheOrderByKey{} ) );
  return pos;
}


size_t LogKeyIndexCache::addToCacheLeftOf(const std::vector<Entry>::const_iterator it, LogATE::Log::Key&& key)
{
  BUT_ASSERT( it != cache_.end() );
  const auto keyIt = data_->find(key);
  if( keyIt == data_->end() )
    return 0;

  const auto nextKnownIt = data_->find(it->key_);
  const auto pos = it->index_ - static_cast<size_t>( std::distance( keyIt, nextKnownIt ) );
  cache_.insert( it, Entry{ std::move(key), pos } );
  BUT_ASSERT( std::is_sorted( cache_.begin(), cache_.end(), CacheOrderByKey{} ) );
  return pos;
}

}
