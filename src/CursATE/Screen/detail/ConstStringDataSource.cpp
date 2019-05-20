#include "CursATE/Screen/detail/ConstStringDataSource.hpp"

namespace CursATE::Screen::detail
{

size_t ConstStringDataSource::index(const Id id) const
{
  return id.value_;
}


size_t ConstStringDataSource::size() const
{
  return entries_.size();
}


But::Optional<ConstStringDataSource::Id> ConstStringDataSource::nearestTo(Id id) const
{
  if( entries_.empty() )
    return  {};
  if( entries_.size() >= id.value_ )
    return Id{ entries_.size()-1u };
  return id;
}


But::Optional<ConstStringDataSource::Id> ConstStringDataSource::first() const
{
  BUT_ASSERT( not entries_.empty() );
  return Id{0};
}


But::Optional<ConstStringDataSource::Id> ConstStringDataSource::last() const
{
  BUT_ASSERT( not entries_.empty() );
  return Id{ entries_.size()-1u };
}


std::map<ConstStringDataSource::Id, std::string> ConstStringDataSource::get(size_t before, Id id, size_t after) const
{
  if( id.value_ > entries_.size() )
    throw std::logic_error{"requested ID in tree, that is out of range"};
  std::map<Id, std::string> out;
  const auto from = ( before > id.value_ ) ? 0u : id.value_ - before;
  const auto to = std::min( entries_.size(), from + before + 1u + after );
  for(auto i=from; i!=to; ++i)
    out[ Id{i} ] = entries_[i];
  return out;
}

}
