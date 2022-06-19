#include "LogATE/Utils/trimFields.hpp"

using LogATE::Tree::Path;
using LogATE::Tree::Node;


namespace LogATE::Utils
{

namespace
{
void trimFrom(nlohmann::json& json, Path const& path)
{
  auto n = &json;
  auto parent = n;
  for(auto& e: path)
  {
    if( n->is_null() )
      return;
    const auto next = n->find( e.name() );  // TODO[array]: add handling of arrays here
    if( next == n->end() )
      return;
    parent = n;
    n = &*next;
  }
  if( parent->is_object() )
    parent->erase( path.last().name() );
  // TODO[array]: add handling of arrays here
}


void trimOneAbsolute(nlohmann::json& json, Path const& path)
{
  trimFrom(json, path);
}


void trimOneRelative(nlohmann::json& json, Path const& path)
{
  if( json.is_null() )
    return;
  trimFrom(json, path);
  if( not json.is_object() && not json.is_array() )
    return;
  for(auto& e: json)
    trimOneRelative(e, path);
}


void trimOneField(nlohmann::json& json, Path const& path)
{
  if( path.empty() )
    return;
  if( path.isAbsolute() )
    trimOneAbsolute(json, path);
  else
    trimOneRelative(json, path);
}
}


Log trimFields(Log const& in, Node::TrimFields const& tf)
{
  if( tf.empty() )
    return in;
  auto json = in.json();
  for(auto& path: tf)
    trimOneField(json, path);
  return Log{ in.key(), std::move(json) };
}

}
