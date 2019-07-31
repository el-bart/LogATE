#include "LogATE/Tree/Logs.hpp"

namespace LogATE::Tree
{

std::set<Log>::iterator Logs::find(Log::Key const& key)
{
  BUT_ASSERT( locked() );
  return logs_.find(key);
}


std::set<Log>::const_iterator Logs::find(Log::Key const& key) const
{
  BUT_ASSERT( locked() );
  return logs_.find(key);
}


void Logs::insert(Log log)
{
  BUT_ASSERT( locked() );
  logs_.insert( std::move(log) );
}


size_t Logs::pruneUpTo(Log::Key const& firstToKeep)
{
  BUT_ASSERT( locked() );
  auto count = 0u;
  while( not empty() && begin()->key() < firstToKeep )
  {
    logs_.erase( begin() );
    ++count;
  }
  return count;
}


std::vector<Log> Logs::range(Log::Key const& beginIt, Log::Key const& endIt) const
{
  BUT_ASSERT( locked() );
  BUT_ASSERT( beginIt <= endIt );
  if( empty() )
    return {};
  const auto low = logs_.lower_bound(beginIt);
  const auto high = logs_.lower_bound(endIt);
  return std::vector<Log>{low, high};
}


std::vector<Log> Logs::from(Log::Key const& first, const size_t count) const
{
  BUT_ASSERT( locked() );
  const auto low = logs_.lower_bound(first);
  std::vector<Log> out;
  out.reserve(count);
  auto i=0u;
  for(auto it=low; it!=end() && i<count; ++i, ++it)
    out.push_back(*it);
  return out;
}


std::vector<Log> Logs::to(Log::Key const& last, const size_t count) const
{
  BUT_ASSERT( locked() );
  const auto highForward = logs_.upper_bound(last);
  const auto high = std::set<Log>::reverse_iterator{highForward};
  std::vector<Log> out;
  out.reserve(count);
  auto i=0u;
  for(auto it=high; it!=rend() && i<count; ++i, ++it)
    out.push_back(*it);
  std::reverse( out.begin(), out.end() );
  return out;
}

}
