#include "LogATE/Tree/detail/LogKeyIndexCache.hpp"
#include <vector>
#include <mutex>
#include <But/assert.hpp>

namespace LogATE::Tree::detail
{

namespace
{
template<typename It>
auto prevIt(It it) { return --it; }
}

size_t LogKeyIndexCache::index(LogATE::Log::Key key)
{
  BUT_ASSERT( std::is_sorted( cache_.begin(), cache_.end(), CacheOrderByKey{} ) );
  const auto cacheLb = std::lower_bound( cache_.begin(), cache_.end(), key, CacheOrderByKey{} );
  if( cacheLb == cache_.end() )
    return addToCacheAtTheEnd( std::move(key) );
  if( cacheLb->key_ == key )
    return cacheLb->index_;
  BUT_ASSERT( key < cacheLb->key_ );

  if( cacheLb == cache_.begin() )
    return addToCacheLeftOf( cacheLb, std::move(key) );
  return addToCacheBetween( prevIt(cacheLb), cacheLb, std::move(key) );
}


void LogKeyIndexCache::updateAfterInsertion(LogATE::Log::Key const& key)
{
  BUT_ASSERT( std::is_sorted( cache_.begin(), cache_.end(), CacheOrderByKey{} ) );
  for(auto it = std::upper_bound( cache_.begin(), cache_.end(), key, CacheOrderByKey{} ); it != cache_.end(); ++it)
    ++it->index_;

  ++insertionCounter_;
  if( insertionCounter_ >= insertEveryNth_ )
  {
    insertionCounter_ = 0;
    index(key);
  }
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


size_t LogKeyIndexCache::addToCacheBetween(std::vector<Entry>::const_iterator low, std::vector<Entry>::const_iterator high, LogATE::Log::Key&& key)
{
  BUT_ASSERT( low != high );
  BUT_ASSERT( low != cache_.end() );
  BUT_ASSERT( low->key_ != key );
  BUT_ASSERT( high->key_ != key );

  auto lowIt = data_->find(low->key_);
  BUT_ASSERT( lowIt != data_->end() );
  auto lowIndex = low->index_;

  auto highIt = data_->find(high->key_);
  BUT_ASSERT( highIt != data_->end() );
  auto highIndex = high->index_;

  while(true)
  {
    ++lowIndex;
    ++lowIt;
    if( lowIt->key() == key )
    {
      cache_.insert( high, Entry{ std::move(key), lowIndex } );
      BUT_ASSERT( std::is_sorted( cache_.begin(), cache_.end(), CacheOrderByKey{} ) );
      return lowIndex;
    }

    --highIndex;
    --highIt;
    if( highIt->key() == key )
    {
      cache_.insert( high, Entry{ std::move(key), highIndex } );
      BUT_ASSERT( std::is_sorted( cache_.begin(), cache_.end(), CacheOrderByKey{} ) );
      return highIndex;
    }
  }
}

}
