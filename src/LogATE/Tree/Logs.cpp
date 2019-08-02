#include "LogATE/Tree/Logs.hpp"

namespace LogATE::Tree
{

namespace
{
template<typename C>
auto findInCollection(C& c, Log::Key const& key)
{
  BUT_ASSERT( std::is_sorted( c.begin(), c.end(), OrderByKey{} ) );
  const auto it = std::lower_bound( c.begin(), c.end(), key, OrderByKey{} );
  if( it == c.end() )
    return c.end();
  if( it->key() != key )
    return c.end();
  return it;
}
}


Logs::iterator Logs::find(Log::Key const& key)
{
  BUT_ASSERT( locked() );
  return findInCollection(logs_, key);
}


Logs::const_iterator Logs::find(Log::Key const& key) const
{
  BUT_ASSERT( locked() );
  return findInCollection(logs_, key);
}


Logs::iterator Logs::lower_bound(Log::Key const& key)
{
  BUT_ASSERT( locked() );
  BUT_ASSERT( std::is_sorted( logs_.begin(), logs_.end(), OrderByKey{} ) );
  return std::lower_bound( logs_.begin(), logs_.end(), key, OrderByKey{} );
}


Logs::const_iterator Logs::lower_bound(Log::Key const& key) const
{
  BUT_ASSERT( locked() );
  BUT_ASSERT( std::is_sorted( logs_.begin(), logs_.end(), OrderByKey{} ) );
  return std::lower_bound( logs_.begin(), logs_.end(), key, OrderByKey{} );
}


Logs::iterator Logs::upper_bound(Log::Key const& key)
{
  BUT_ASSERT( locked() );
  BUT_ASSERT( std::is_sorted( logs_.begin(), logs_.end(), OrderByKey{} ) );
  return std::upper_bound( logs_.begin(), logs_.end(), key, OrderByKey{} );
}


Logs::const_iterator Logs::upper_bound(Log::Key const& key) const
{
  BUT_ASSERT( locked() );
  BUT_ASSERT( std::is_sorted( logs_.begin(), logs_.end(), OrderByKey{} ) );
  return std::upper_bound( logs_.begin(), logs_.end(), key, OrderByKey{} );
}


void Logs::insert(Log log)
{
  BUT_ASSERT( locked() );
  BUT_ASSERT( std::is_sorted( begin(), end(), OrderByKey{} ) );
  if( logs_.empty() )
  {
    logs_.push_back(std::move(log));
    return;
  }
  if( logs_.back().key() < log.key() )
  {
    logs_.push_back(std::move(log));
    return;
  }
  auto it = std::upper_bound( begin(), end(), log, OrderByKey{} );
  logs_.insert(it, std::move(log));
  BUT_ASSERT( std::is_sorted( begin(), end(), OrderByKey{} ) );
}


size_t Logs::pruneUpTo(Log::Key const& firstToKeep)
{
  BUT_ASSERT( locked() );
  BUT_ASSERT( std::is_sorted( begin(), end(), OrderByKey{} ) );
  const auto it = std::lower_bound( begin(), end(), firstToKeep, OrderByKey{} );
  const auto count = std::distance( begin(), it );
  logs_.erase( begin(), it );
  return count;
}


std::vector<Log> Logs::range(Log::Key const& beginIt, Log::Key const& endIt) const
{
  BUT_ASSERT( locked() );
  BUT_ASSERT( std::is_sorted( begin(), end(), OrderByKey{} ) );
  auto low = std::lower_bound( begin(), end(), beginIt, OrderByKey{} );
  if( low == end() )
    low = begin();
  const auto high = std::lower_bound( begin(), end(), endIt, OrderByKey{} );
  return std::vector<Log>{low, high};
}


std::vector<Log> Logs::from(Log::Key const& first, const size_t count) const
{
  BUT_ASSERT( locked() );
  BUT_ASSERT( std::is_sorted( begin(), end(), OrderByKey{} ) );
  const auto low = std::lower_bound( begin(), end(), first, OrderByKey{} );
  const auto maxCount = std::distance(low, end());
  const auto elements = std::min<size_t>(maxCount, count);
  return std::vector<Log>{low, low+elements};
}


std::vector<Log> Logs::to(Log::Key const& last, const size_t count) const
{
  BUT_ASSERT( locked() );
  BUT_ASSERT( std::is_sorted( begin(), end(), OrderByKey{} ) );
  const auto high = std::upper_bound( begin(), end(), last, OrderByKey{} );
  const auto maxCount = std::distance(begin(), high);
  const auto elements = std::min<size_t>(maxCount, count);
  return std::vector<Log>{high-elements, high};
}

}
