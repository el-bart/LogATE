#include "CursATE/Screen/detail/LogDataSource.hpp"

namespace CursATE::Screen::detail
{

namespace
{
auto id2key(LogDataSource::Id const& id) { return LogATE::Log::Key{ id.value_ }; }

auto key2id(LogATE::Log::Key const& key) { return LogDataSource::Id{ key.str() }; }

auto data(LogATE::Tree::NodeShPtr const& node, size_t before, LogDataSource::Id id, size_t after)
{
  const auto& ll = node->logs().withLock();
  auto pre = ll->to( id2key(id), before+1 );
  auto post = ll->from( id2key(id), after+1 );
  return std::make_pair( std::move(pre), std::move(post) );
}
}


size_t LogDataSource::index(Id const& id) const
{
  auto node = node_.lock();
  if(not node)
    return {};
  auto ll = node->logs().withLock();
  if( ll->empty() )
    return 0;

  const auto it = std::lower_bound( ll->begin(), ll->end(), id2key(id), LogATE::OrderByKey{} );
  if( it == ll->end() )
    return 0;
  if( it->key() != id2key(id) )
    return 0;
  return std::distance( ll->begin(), it );
}

size_t LogDataSource::size() const
{
  const auto node = node_.lock();
  if(not node)
    return 0;
  return node->logs().withLock()->size();
}


But::Optional<LogDataSource::Id> LogDataSource::nearestTo(Id const& id) const
{
  auto node = node_.lock();
  if(not node)
    return {};
  auto ll = node->logs().withLock();
  if( ll->empty() )
    return {};

  const auto it = std::lower_bound( ll->begin(), ll->end(), id2key(id), LogATE::OrderByKey{} );
  if( it == ll->end() )
    return key2id( ll->last().key() );
  if( it == ll->begin() )
    return key2id( it->key() );
  return closest( id, key2id( (it-1)->key() ), key2id( it->key() ) );
}

But::Optional<LogDataSource::Id> LogDataSource::first() const
{
  const auto node = node_.lock();
  if(not node)
    return {};
  const auto& ll = node->logs().withLock();
  if( ll->empty() )
    return {};
  return key2id( ll->first().key() );
}

But::Optional<LogDataSource::Id> LogDataSource::last() const
{
  const auto node = node_.lock();
  if(not node)
    return {};
  const auto& ll = node->logs().withLock();
  if( ll->empty() )
    return {};
  return key2id( ll->last().key() );
}


namespace
{
template<typename F>
std::string logToString(F const& f, LogATE::Log const& log)
{
  try
  {
    return f(log);
  }
  catch(std::exception const& ex)
  {
    std::stringstream ss;
    ss << log.sequenceNumber().value_ << " << log printing failed: " << ex.what() << " >>";
    return ss.str();
  }
}
}

std::map<LogDataSource::Id, std::string> LogDataSource::get(size_t before, Id const& id, size_t after) const
{
  BUT_ASSERT(log2str_);
  const auto node = node_.lock();
  if(not node)
    return {};
  auto [pre, post] = data( LogATE::Tree::NodeShPtr{node}, before, id, after);
  std::map<Id, std::string> out;
  for(auto& set: { pre, post })
    for(auto& log: set)
      out[ key2id( log.key() ) ] = logToString(log2str_, log);
  return out;
}

}
