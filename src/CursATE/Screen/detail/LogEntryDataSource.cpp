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
  if( entries_.empty() )
    return {};
  return Id{0};
}


But::Optional<LogEntryDataSource::Id> LogEntryDataSource::last() const
{
  if( entries_.empty() )
    return {};
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
But::Optional<std::string> optPrintable(PrintableStringConverter const& printable, But::Optional<std::string> in)
{
  if(not in)
    return {};
  return printable(*in);
}

auto indent(std::string const& prefix) { return prefix + "  "; }


auto extend(Path const& path, std::string key)
{
  auto newPathData = path.data();
  newPathData.emplace_back( std::move(key) );
  return Path{ std::move(newPathData), path.isAbsolute() };
}


auto extend(Path const& path, std::string const& key, size_t index)
{
  std::stringstream ss;
  ss << key << '[' << index << ']';
  return extend( path, ss.str() );
}


template<typename C>
void appendTree(nlohmann::json const& log, C& out, Path const& path, PrintableStringConverter const& printable, std::string const& prefix);


template<typename C>
void appendTreeObject(nlohmann::json const& log, C& out, Path const& path, PrintableStringConverter const& printable, std::string const& prefix)
{
  BUT_ASSERT( log.is_object() );

  for( auto& [k,v]: log.items() )
  {
    std::stringstream ssText;
    auto key = printable(k);
    ssText << prefix << key;
    if( v.is_array() )
    {
      ssText << "[]";
      key += "[]";
    }
    auto value = optPrintable( printable, value2str(v) );
    if(value)
      ssText << ": " << *value;

    auto newPath = extend(path, key);
    using E = typename C::value_type;
    out.push_back( E{newPath, ssText.str(), value} );

    if(not value)
      appendTree( v, out, std::move(newPath), printable, indent(prefix) );
  }
}


template<typename C>
void appendTreeArray(nlohmann::json const& log, C& out, Path const& path, PrintableStringConverter const& printable, std::string const& prefix)
{
  BUT_ASSERT( log.is_array() );
  BUT_ASSERT( not path.empty() && "root-level arrays are not allowed" );
  auto pathTemplateData = path.data();
  pathTemplateData.pop_back();
  auto pathTemplate = Path{ std::move(pathTemplateData), path.isAbsolute() };

  auto index = size_t{0};
  for(auto& e: log)
  {
    std::stringstream ssText;
    ssText << prefix << '[' << index << ']';
    auto value = optPrintable( printable, value2str(e) );
    if(value)
      ssText << ": " << *value;

    auto newPath = extend(pathTemplate, printable( path.last().name() ), index);
    using E = typename C::value_type;
    out.push_back( E{newPath, ssText.str(), value} );
    ++index;

    if(not value)
      appendTree( e, out, std::move(newPath), printable, indent(prefix) );
  }
}


template<typename C>
void appendTree(nlohmann::json const& log, C& out, Path const& path, PrintableStringConverter const& printable, std::string const& prefix)
{
  if( log.is_null() )
    return;
  if( log.is_object() )
  {
    appendTreeObject(log, out, path, printable, prefix);
    return;
  }
  if( log.is_array() )
  {
    appendTreeArray(log, out, path, printable, prefix);
    return;
  }
  BUT_ASSERT("top level element has to be an object");
  throw std::logic_error{"top level element has to be an object"};
}
}

std::vector<LogEntryDataSource::Entry> LogEntryDataSource::constructEntries(LogATE::Log const& log) const
{
  if( not log.json().is_object() )
    throw std::logic_error{"only objects can be root-level elements"};
  std::vector<Entry> out;
  appendTree( log.json(), out, Path{{"."}}, printable_, "" );
  return out;
}

}
