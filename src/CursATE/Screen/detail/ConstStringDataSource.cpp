#include "CursATE/Screen/detail/ConstStringDataSource.hpp"
#include <boost/lexical_cast.hpp>

namespace CursATE::Screen::detail
{

size_t ConstStringDataSource::index(Id const& id) const
{
  return boost::lexical_cast<size_t>(id.value_);
}


size_t ConstStringDataSource::size() const
{
  return entries_.size();
}


But::Optional<ConstStringDataSource::Id> ConstStringDataSource::nearestTo(Id const& id) const
{
  if( entries_.empty() )
    return  {};
  if( entries_.size() >= boost::lexical_cast<size_t>(id.value_) )
    return Id{ std::to_string( entries_.size()-1u ) };
  return id;
}


But::Optional<ConstStringDataSource::Id> ConstStringDataSource::first() const
{
  if( entries_.empty() )
    return {};
  return Id{0};
}


But::Optional<ConstStringDataSource::Id> ConstStringDataSource::last() const
{
  if( entries_.empty() )
    return {};
  return Id{ std::to_string( entries_.size()-1u ) };
}


std::map<ConstStringDataSource::Id, std::string> ConstStringDataSource::get(size_t before, Id const& id, size_t after) const
{
  const auto idNum = boost::lexical_cast<size_t>(id.value_);
  if( idNum > entries_.size() )
    throw std::logic_error{"requested ID in tree, that is out of range"};
  std::map<Id, std::string> out;
  const auto from = ( before > idNum ) ? 0u : idNum - before;
  const auto to = std::min( entries_.size(), from + before + 1u + after );
  for(auto i=from; i!=to; ++i)
    out[ Id{ std::to_string(i) } ] = entries_[i];
  return out;
}

}
