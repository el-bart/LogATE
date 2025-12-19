#include "LogATE/Tree/KeyExtractor.hpp"
#include "LogATE/Utils/value2str.hpp"
#include <But/assert.hpp>

namespace LogATE::Tree
{

KeyExtractor::KeyExtractor(Path path, SourceFormat const sourceFormat):
  path_{ std::move(path) },
  sourceFormat_{sourceFormat}
{
  if( path_.empty() )
    BUT_THROW(InvalidKeyPath, "key path cannot be empty");
  if( not path_.isAbsolute() )
    BUT_THROW(InvalidKeyPath, "key path must be absolute: " << path_.str() );
  if( not path_.isUnique() )
    BUT_THROW(InvalidKeyPath, "key path must be unique (no wildcards allowed): " << path_.str() );
  // TODO[array]: support for arrays shall be added over time for keys as well...
  for(auto& e: path.data())
    if( e.isArray() )
      throw std::logic_error{"arrays in key path are not supported atm"};
}


namespace
{
auto notFound()   { return "<< key not found >>"; }
auto invalidKey() { return "<< invalid key >>"; }


nlohmann::json const* getKeyNode(nlohmann::json const& json, Path const& path)
{
  auto* node = &json;
  for(auto pit = path.begin(); pit != path.end(); ++pit)
  {
    auto it = node->find( pit->name() );    // TODO[array]: ok only for non-array elements!
    if( it == node->end() )
      return nullptr;
    node = &*it;
  }
  return node;
}


inline std::string getKeyAny(nlohmann::json const& json, Path const& path)
{
  auto const* node = getKeyNode(json, path);
  if(not node)
    return notFound();
  auto opt = Utils::value2str(*node);
  if(not opt)
    return invalidKey();
  return std::move(*opt);
}


inline std::string getKeyStr(nlohmann::json const& json, Path const& path)
{
  auto const* node = getKeyNode(json, path);
  if(not node)
    return notFound();
  if( not node->is_string() )
    return invalidKey();
  return node->get<std::string>();
}


inline std::optional<int64_t> toOptInt(std::string const& in)
{
    try
    {
      auto first = true;
      for(auto c: in)
      {
        if(first && c == '-')
        {
          first = false;
          continue;
        }
        first = false;
        if(not isdigit(c))
          return {};
      }

      return std::stoll(in);
    }
    catch(...)
    {
      return {};
    }
}


inline std::optional<double> toOptFloat(std::string const& in)
{
    try
    {
      auto first = true;
      auto hasDot = false;
      for(auto c: in)
      {
        if(first && c == '-')
        {
          first = false;
          continue;
        }
        first = false;
        if(c == '.')
        {
          if(hasDot)
            return {};
          hasDot = true;
          continue;
        }
        if(not isdigit(c))
          return {};
      }

      return std::stod(in);
    }
    catch(...)
    {
      return {};
    }
}


std::optional<int64_t> getKeyInt(nlohmann::json const& json, Path const& path)
{
  auto const* node = getKeyNode(json, path);
  if(not node)
    return {};
  if( node->is_number_integer() )
    return node->get<int64_t>();
  if( node->is_string() )
    return toOptInt( node->get<std::string>() );
  return {};
}


std::optional<double> getKeyFloat(nlohmann::json const& json, Path const& path)
{
  auto const* node = getKeyNode(json, path);
  if(not node)
    return {};
  if( node->is_number_float() )
    return node->get<double>();
  if( node->is_number_integer() ) // in case some lib encoded '1.0' as '1'
    return node->get<int64_t>();
  if( node->is_string() )
    return toOptFloat( node->get<std::string>() );
  return {};
}


inline std::string ensureIso(std::string&& key)
{
  if( key.size() != sizeof("2025-07-02T22:06:13.123456789Z")-1u )
    return invalidKey();
  // TODO: more robust key checks one day...
  return key;
}


std::string fromUnixTs(std::optional<int64_t> const ts, int64_t const div)
{
  if(not ts)
    return invalidKey();
  BUT_ASSERT(div > 0u);
  auto const secs  = *ts / div;
  auto const fract = *ts % div;

  tm dt;
  if( not gmtime_r(&secs, &dt) )
    return invalidKey();
  char buf[1024]; // keep it large in case year is way out of ~2k range
  snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%09ldZ",
        dt.tm_year + 1900,
        dt.tm_mon + 1,
        dt.tm_mday,
        dt.tm_hour,
        dt.tm_min,
        dt.tm_sec,
        fract * ( 1'000'000'000 / div )
      );
  return buf;
}

inline std::string fromUnixTsFloat(std::optional<double> const ts)
{
  if(not ts)
    return invalidKey();
  return fromUnixTs( static_cast<int64_t>( *ts * 1'000'000'000 ), 1'000'000'000 );
}
} // unnamed namespace


std::string KeyExtractor::operator()(nlohmann::json const& in) const
{
  switch(sourceFormat_)
  {
    case KeyExtractor::SourceFormat::Raw:        return getKeyAny(in, path_);
    case KeyExtractor::SourceFormat::ISO8601_ns: return ensureIso( getKeyStr(in, path_) );
    case KeyExtractor::SourceFormat::UNIX:       return fromUnixTs( getKeyInt(in, path_), 1 );
    case KeyExtractor::SourceFormat::UNIX_ms:    return fromUnixTs( getKeyInt(in, path_), 1'000 );
    case KeyExtractor::SourceFormat::UNIX_us:    return fromUnixTs( getKeyInt(in, path_), 1'000'000 );
    case KeyExtractor::SourceFormat::UNIX_ns:    return fromUnixTs( getKeyInt(in, path_), 1'000'000'000 );
    case KeyExtractor::SourceFormat::UNIX_float: return fromUnixTsFloat( getKeyFloat(in, path_) );
  }
  BUT_THROW(KeyExtractor::Error, "invalid format requested");
}

}
