#include "CursATE/Screen/detail/LogEntryDataSource.hpp"
#include "LogATE/Utils/value2str.hpp"

using LogATE::Tree::Path;
using LogATE::Utils::value2str;

namespace CursATE::Screen::detail
{

size_t LogEntryDataSource::size() const
{
  return entries_.size();
}


But::Optional<LogEntryDataSource::Id> LogEntryDataSource::nearestTo(Id id) const
{
  if( entries_.empty() )
    return  {};
  if( entries_.size() >= id.value_ )
    return Id{ entries_.size()-1u };
  return id;
}


But::Optional<LogEntryDataSource::Id> LogEntryDataSource::first() const
{
  BUT_ASSERT( not entries_.empty() );
  return Id{0};
}


But::Optional<LogEntryDataSource::Id> LogEntryDataSource::last() const
{
  BUT_ASSERT( not entries_.empty() );
  return Id{ entries_.size()-1u };
}


std::map<LogEntryDataSource::Id, std::string> LogEntryDataSource::get(size_t before, Id id, size_t after) const
{
  if( id.value_ > entries_.size() )
    throw std::logic_error{"requested ID in tree, that is out of range"};
  std::map<Id, std::string> out;
  const auto from = ( before > id.value_ ) ? 0u : id.value_ - before;
  const auto to = std::min( entries_.size(), from + before + 1u + after );
  for(auto i=from; i!=to; ++i)
    out[ Id{i} ] = entries_[i].text_;
  return out;
}


LogATE::Tree::Path LogEntryDataSource::id2path(const Id id) const
{
  BUT_ASSERT( id.value_ < entries_.size() );
  return entries_[id.value_].path_;
}

But::Optional<std::string> LogEntryDataSource::id2value(const Id id) const
{
  BUT_ASSERT( id.value_ < entries_.size() );
  return entries_[id.value_].value_;
}


namespace
{
template<typename C>
void appendTree(nlohmann::json const& log, C& out, Path const& path, std::string const& prefix)
{
  const auto newPrefix = prefix + "  ";
  using E = typename C::value_type;
  for( auto& e: log.items() )
  {
    auto newPathData = path.data();
    newPathData.push_back( e.key() );
    auto value = value2str( e.value() );
    auto text = prefix + e.key();
    if(value)
      text += ": " + *value;
    out.push_back( E{ Path{newPathData}, std::move(text), value } );
    if(not value)
      appendTree(e.value(), out, Path{newPathData}, newPrefix);
  }
}
}

std::vector<LogEntryDataSource::Entry> LogEntryDataSource::constructEntries(LogATE::Log const& log) const
{
  std::vector<Entry> out;
  appendTree(log.json(), out, Path{{"."}}, "");
  return out;
}

}
