#include "CursATE/Screen/detail/Marks.hpp"
#include <But/Optional.hpp>

namespace CursATE::Screen::detail
{

But::Optional<Marks::Entry> Marks::find(const char k) const
{
  const auto it = data_.find(k);
  if( it == end(data_) )
    return {};
  return it->second;
}


void Marks::prune()
{
  auto it=begin(data_);
  while( it!=end(data_) )
  {
    auto tmp = it++;
    if( not tmp->second.node_.lock() )
      data_.erase(tmp);
  }
}


void Marks::insert(const char c, Entry e)
{
  data_.erase(c);
  data_.insert( std::make_pair(c, std::move(e)) );
}

}
