#include "CursATE/Screen/detail/LogEntryDataSource.hpp"
#include "LogATE/Utils/value2str.hpp"
#include <boost/lexical_cast.hpp>

using LogATE::Tree::Path;
using LogATE::Utils::value2str;
using LogATE::Utils::PrintableStringConverter;

namespace CursATE::Screen::detail
{

size_t LogEntryDataSource::index(Id const& id) const
{
  return boost::lexical_cast<size_t>(id.value_);
}


size_t LogEntryDataSource::size() const
{
  return entries_.size();
}


But::Optional<LogEntryDataSource::Id> LogEntryDataSource::nearestTo(Id const& id) const
{
  if( entries_.empty() )
    return  {};
  if( entries_.size() >= boost::lexical_cast<size_t>(id.value_) )
    return Id{entries_.size()-1u};
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
  return Id{entries_.size()-1u};
}


std::map<LogEntryDataSource::Id, std::string> LogEntryDataSource::get(size_t before, Id const& id, size_t after) const
{
  const auto idNum = boost::lexical_cast<size_t>(id.value_);
  if( idNum > entries_.size() )
    throw std::logic_error{"requested ID in tree, that is out of range"};
  std::map<Id, std::string> out;
  const auto from = ( before > idNum ) ? 0u : idNum - before;
  const auto to = std::min( entries_.size(), from + before + 1u + after );
  for(auto i=from; i!=to; ++i)
    out[ Id{i} ] = entries_[i].text_;
  return out;
}


LogATE::Tree::Path LogEntryDataSource::id2path(Id const& id) const
{
  const auto idNum = boost::lexical_cast<size_t>(id.value_);
  BUT_ASSERT( idNum < entries_.size() );
  return entries_[idNum].path_;
}

But::Optional<std::string> LogEntryDataSource::id2value(Id const& id) const
{
  const auto idNum = boost::lexical_cast<size_t>(id.value_);
  BUT_ASSERT( idNum < entries_.size() );
  return entries_[idNum].value_;
}


namespace
{
But::Optional<std::string> optPrintable(PrintableStringConverter const& printable, But::Optional<std::string> const& in)
{
  if(not in)
    return {};
  return printable(*in);
}

template<typename C>
void appendTree(nlohmann::json const& log, C& out, Path const& path, PrintableStringConverter const& printable, std::string const& prefix)
{
  const auto newPrefix = prefix + "  ";
  using E = typename C::value_type;
  for( auto& e: log.items() )
  {
    auto newPathData = path.data();
    newPathData.emplace_back( printable( e.key() ) );
    auto value = value2str( e.value() );
    auto text = prefix + printable( e.key() );
    if(value)
      text += ": " + printable( *value );
    out.push_back( E{ Path{newPathData, path.absolute()}, std::move(text), optPrintable(printable, value) } );
    if(not value)
      appendTree(e.value(), out, Path{newPathData, path.absolute()}, printable, newPrefix);
  }
}
}

std::vector<LogEntryDataSource::Entry> LogEntryDataSource::constructEntries(LogATE::Log const& log) const
{
  std::vector<Entry> out;
  appendTree(log.json(), out, Path{{"."}}, printable_, "");
  return out;
}

}
