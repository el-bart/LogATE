#include "LogATE/Tree/Logs.hpp"

namespace LogATE::Tree
{

namespace
{
struct OrderBySequenceNumber
{
  auto operator()(Log const&            lhs, Log const&            rhs) const { return lhs.sn_ < rhs.sn_; }
  auto operator()(Log const&            lhs, SequenceNumber const& rhs) const { return lhs.sn_ < rhs; }
  auto operator()(SequenceNumber const& lhs, Log const&            rhs) const { return lhs     < rhs.sn_; }
  auto operator()(SequenceNumber const& lhs, SequenceNumber const& rhs) const { return lhs     < rhs; }
};
}

void Logs::insert(Log log)
{
  BUT_ASSERT( locked() );
  BUT_ASSERT( std::is_sorted( begin(), end(), OrderBySequenceNumber{} ) );
  if( logs_.empty() )
  {
    logs_.push_back(std::move(log));
    return;
  }
  if( logs_.back().sn_ < log.sn_ )
  {
    logs_.push_back(std::move(log));
    return;
  }
  auto it = std::upper_bound( begin(), end(), log, OrderBySequenceNumber{} );
  logs_.insert(it, std::move(log));
  BUT_ASSERT( std::is_sorted( begin(), end(), OrderBySequenceNumber{} ) );
}

size_t Logs::pruneUpTo(const SequenceNumber firstToKeep)
{
  BUT_ASSERT( locked() );
  BUT_ASSERT( std::is_sorted( begin(), end(), OrderBySequenceNumber{} ) );
  const auto it = std::lower_bound( begin(), end(), firstToKeep, OrderBySequenceNumber{} );
  const auto count = std::distance( begin(), it );
  logs_.erase( begin(), it );
  return count;
}

std::vector<Log> Logs::range(const SequenceNumber beginIt, const SequenceNumber endIt) const
{
  BUT_ASSERT( locked() );
  BUT_ASSERT( std::is_sorted( begin(), end(), OrderBySequenceNumber{} ) );
  auto low = std::lower_bound( begin(), end(), beginIt, OrderBySequenceNumber{} );
  if( low == end() )
    low = begin();
  const auto high = std::lower_bound( begin(), end(), endIt, OrderBySequenceNumber{} );
  return std::vector<Log>{low, high};
}

std::vector<Log> Logs::from(const SequenceNumber first, const size_t count) const
{
  BUT_ASSERT( locked() );
  BUT_ASSERT( std::is_sorted( begin(), end(), OrderBySequenceNumber{} ) );
  const auto low = std::lower_bound( begin(), end(), first, OrderBySequenceNumber{} );
  const auto maxCount = std::distance(low, end());
  const auto elements = std::min<size_t>(maxCount, count);
  return std::vector<Log>{low, low+elements};
}

std::vector<Log> Logs::to(const SequenceNumber last, const size_t count) const
{
  BUT_ASSERT( locked() );
  BUT_ASSERT( std::is_sorted( begin(), end(), OrderBySequenceNumber{} ) );
  const auto high = std::upper_bound( begin(), end(), last, OrderBySequenceNumber{} );
  const auto maxCount = std::distance(begin(), high);
  const auto elements = std::min<size_t>(maxCount, count);
  return std::vector<Log>{high-elements, high};
}

}