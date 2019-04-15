#include "CursATE/detail/LogDataSource.hpp"

namespace CursATE::detail
{

namespace
{
auto id2sn(const LogDataSource::Id id) { return LogATE::SequenceNumber{ id.value_ }; }

auto sn2id(const LogATE::SequenceNumber sn) { return LogDataSource::Id{ sn.value_ }; }

auto data(LogATE::Tree::NodeShPtr const& node, size_t before, LogDataSource::Id id, size_t after)
{
  const auto& ll = node->logs().withLock();
  auto pre = ll->to( id2sn(id), before );
  auto post = ll->from( id2sn(id), after );
  return std::make_pair( std::move(pre), std::move(post) );
}
}

But::Optional<LogDataSource::Id> LogDataSource::first() const
{
  const auto& ll = node_->logs().withLock();
  if( ll->empty() )
    return {};
  return Id{ ll->first().sn_.value_ };
}

But::Optional<LogDataSource::Id> LogDataSource::last() const
{
  const auto& ll = node_->logs().withLock();
  if( ll->empty() )
    return {};
  return Id{ ll->last().sn_.value_ };
}

std::map<LogDataSource::Id, std::string> LogDataSource::get(size_t before, Id id, size_t after) const
{
  BUT_ASSERT(log2str_);
  auto [pre, post] = data(node_, before, id, after);
  std::map<Id, std::string> out;
  for(auto& set: { pre, post })
    for(auto& log: set)
      out[ sn2id(log.sn_) ] = log2str_(log);
  return out;
}

}
