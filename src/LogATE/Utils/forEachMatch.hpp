#pragma once
#include "LogATE/Tree/Path.hpp"
#include <nlohmann/json.hpp>

namespace LogATE::Utils
{

namespace detail
{

template<typename F>
bool forEachMatchImpl(nlohmann::json const& root, Tree::Path const& path, F&& f)
{
  auto n = &root;
  for(auto& e: path)
  {
    auto it = n->find( e.name() ); // TODO: arrays....
    if( it == n->end() )
      return true;
    if( it->is_null() )
      return true;
    n = &*it;
  }
  return f(*n);
}


template<typename F>
bool forEachMatchAbsolute(nlohmann::json const& root, Tree::Path const& path, F&& f)
{
  return forEachMatchImpl(root, path, f);
}


template<typename F>
bool forEachMatchRelative(nlohmann::json const& root, Tree::Path const& path, F&& f)
{
  if( not forEachMatchImpl(root, path, f) )
    return false;

  if( root.is_object() )
  {
    for(auto& node: root.items())
      if( not forEachMatchRelative(node.value(), path, f) )
        return false;
  }

  // TODO: arrays

  return true;
}

}


// if 'f(node)' returns 'false', operation is stopped and 'false' is returned. 'true' otherwise.
template<typename F>
bool forEachMatch(nlohmann::json const& root, Tree::Path const& path, F&& f)
{
  if( path.empty() )
  {
    if( path.absolute() )
      return f(root);
    return true;
  }

  if( path.absolute() )
    return detail::forEachMatchAbsolute(root, path, f);
  else
    return detail::forEachMatchRelative(root, path, f);
}

}
