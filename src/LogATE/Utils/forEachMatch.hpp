#pragma once
#include "LogATE/Tree/Path.hpp"
#include <nlohmann/json.hpp>

namespace LogATE::Utils
{

namespace detail
{

template<typename F>
bool handleEndOfPath(nlohmann::json const& node, F&& f)
{
  if( not node.is_null() )
    return f(node);
  return true;
}


inline nlohmann::json const* findByName(nlohmann::json const& n, Tree::Path::Entry const& e)
{
  auto it = n.find( e.name() );
  if( it == n.end() )
    return nullptr;
  if( it->is_null() )
    return nullptr;
  return &*it;
}


template<typename F>
bool forEachMatchOne(nlohmann::json const& root, Tree::Path::Data::const_iterator begin, Tree::Path::Data::const_iterator end, F&& f)
{
  if( begin == end )
    return handleEndOfPath(root, f);

  auto n = &root;
  for(auto it=begin; it!=end; ++it)
  {
    n = findByName(*n, *it);
    if( n == nullptr )
      return true;

    if( it->isArray() )
    {
      if( not n->is_array() )
        return true;
      if( it->hasIndex() )
      {
        if( n->size() <= it->index() )
          return true;
        n = &(*n)[ it->index() ];
      }
      else
      {
        for(auto& elem: *n)
        {
          // TODO: if false...
          forEachMatchOne(elem, it+1, end, f);
        }
        return true; // TODO...
      }

      if( n == nullptr )
        return true;
    }
    else
      if( n->is_array() )
        return true;
  }

  BUT_ASSERT(n);
  return handleEndOfPath(*n, f);
}


template<typename F>
bool forEachMatchOne(nlohmann::json const& root, Tree::Path const& path, F&& f)
{
  return forEachMatchOne(root, path.begin(), path.end(), f);
}


template<typename F>
bool forEachMatchAbsolute(nlohmann::json const& root, Tree::Path const& path, F&& f)
{
  return forEachMatchOne(root, path, f);
}


template<typename F>
bool forEachMatchRelative(nlohmann::json const& root, Tree::Path const& path, F&& f)
{
  if( not forEachMatchOne(root, path, f) )
    return false;

  if( root.is_object() )
  {
    for(auto& node: root.items())
      if( not forEachMatchRelative(node.value(), path, f) )
        return false;
  }

  // TODO[array]: top-level arrays are to be supported here as well

  return true;
}

}


// if 'f(node)' returns 'false', operation is stopped and 'false' is returned. 'true' otherwise.
template<typename F>
bool forEachMatch(nlohmann::json const& root, Tree::Path const& path, F&& f)
{
  if( path.empty() )
  {
    if( path.isAbsolute() )
      return f(root);
    return true;
  }

  if( path.isAbsolute() )
    return detail::forEachMatchAbsolute(root, path, f);
  else
    return detail::forEachMatchRelative(root, path, f);
}

}
