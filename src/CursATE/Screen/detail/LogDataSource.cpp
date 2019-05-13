#include "CursATE/Screen/detail/LogDataSource.hpp"

namespace CursATE::Screen::detail
{

namespace
{
auto id2sn(const LogDataSource::Id id) { return LogATE::SequenceNumber{ id.value_ }; }

auto sn2id(const LogATE::SequenceNumber sn) { return LogDataSource::Id{ sn.value_ }; }

auto data(LogATE::Tree::NodeShPtr const& node, size_t before, LogDataSource::Id id, size_t after)
{
  const auto& ll = node->logs().withLock();
  auto pre = ll->to( id2sn(id), before+1 );
  auto post = ll->from( id2sn(id), after+1 );
  return std::make_pair( std::move(pre), std::move(post) );
}

struct OrderBySequenceNumber final
{
  using Log = LogATE::Log;
  using SN = LogATE::SequenceNumber;
  auto operator()(Log const& lhs, Log const& rhs) const { return lhs.sequenceNumber() < rhs.sequenceNumber(); }
  auto operator()(Log const& lhs, SN const&  rhs) const { return lhs.sequenceNumber() < rhs; }
  auto operator()(SN const&  lhs, Log const& rhs) const { return lhs                  < rhs.sequenceNumber(); }
  auto operator()(SN const&  lhs, SN const&  rhs) const { return lhs                  < rhs; }
};
}


size_t LogDataSource::index(const Id id) const
{
  auto node = node_.lock();
  if(not node)
    return {};
  auto ll = node->logs().withLock();
  if( ll->empty() )
    return 0;

  const auto it = std::lower_bound( ll->begin(), ll->end(), id2sn(id), OrderBySequenceNumber{} );
  if( it == ll->end() )
    return ll->size()-1;
  return std::distance( ll->begin(), it );
}

size_t LogDataSource::size() const
{
  const auto node = node_.lock();
  if(not node)
    return 0;
  return node->logs().withLock()->size();
}


But::Optional<LogDataSource::Id> LogDataSource::nearestTo(const Id id) const
{
  auto node = node_.lock();
  if(not node)
    return {};
  auto ll = node->logs().withLock();
  if( ll->empty() )
    return {};

  const auto it = std::lower_bound( ll->begin(), ll->end(), id2sn(id), OrderBySequenceNumber{} );
  if( it == ll->end() )
    return sn2id( ll->last().sequenceNumber() );
  if( it == ll->begin() )
    return sn2id(it->sequenceNumber());
  return closest(id, sn2id( (it-1)->sequenceNumber() ), sn2id(it->sequenceNumber()));
}

But::Optional<LogDataSource::Id> LogDataSource::first() const
{
  const auto node = node_.lock();
  if(not node)
    return {};
  const auto& ll = node->logs().withLock();
  if( ll->empty() )
    return {};
  return Id{ ll->first().sequenceNumber().value_ };
}

But::Optional<LogDataSource::Id> LogDataSource::last() const
{
  const auto node = node_.lock();
  if(not node)
    return {};
  const auto& ll = node->logs().withLock();
  if( ll->empty() )
    return {};
  return Id{ ll->last().sequenceNumber().value_ };
}

std::map<LogDataSource::Id, std::string> LogDataSource::get(size_t before, Id id, size_t after) const
{
  BUT_ASSERT(log2str_);
  const auto node = node_.lock();
  if(not node)
    return {};
  auto [pre, post] = data( LogATE::Tree::NodeShPtr{node}, before, id, after);
  std::map<Id, std::string> out;
  for(auto& set: { pre, post })
    for(auto& log: set)
      out[ sn2id(log.sequenceNumber()) ] = log2str_(log);
  return out;
}

}
