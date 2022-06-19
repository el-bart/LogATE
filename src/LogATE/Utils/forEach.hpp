#pragma once
#include <nlohmann/json.hpp>
#include <But/assert.hpp>

namespace LogATE::Utils
{

namespace detail
{

template<typename F>
bool forEachImpl(nlohmann::json const& root, F&& f);


template<typename F>
bool forEachSubnode(nlohmann::json const& node, F&& f)
{
  BUT_ASSERT( not node.is_null() );
  if( node.is_object() || node.is_array() )
    for(auto& e: node)
      if( not forEachImpl(e, f) )
        return false;
  return true;
}


template<typename F>
bool forEachImpl(nlohmann::json const& node, F&& f)
{
  if( node.is_null() )
    return true;
  if( not f(node) )
    return false;
  return forEachSubnode(node, f);
}

}


// if 'f(node)' returns 'false', operation is stopped and 'false' is returned. 'true' otherwise.
template<typename F>
bool forEach(nlohmann::json const& root, F&& f)
{
  if( root.is_null() )
    return true;
  // note that processing 'root' node is deliberately skipped here, as it does not really make much sense...
  return detail::forEachSubnode(root, f);
}

}
