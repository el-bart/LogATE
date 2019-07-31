#include "LogATE/Utils/trimFields.hpp"

using LogATE::Tree::Path;
using LogATE::Tree::Node;


namespace LogATE::Utils
{

namespace
{
struct PathDecomposition
{
  explicit PathDecomposition(Path const& p):
    begin_{ p.begin() },
    end_{ p.begin() }
  {
    BUT_ASSERT( not p.empty() );
    std::advance(end_, p.data().size()-1);
    finalNode_ = &*end_;
  }

  std::vector<std::string>::const_iterator begin_;
  std::vector<std::string>::const_iterator end_;
  std::string const* finalNode_{nullptr};
};

template<typename It>
void trimFrom(nlohmann::json& json, It beginPath, It endPath, std::string const& finalNode)
{
  auto n = &json;
  for(auto it=beginPath; it!=endPath; ++it)
  {
    if( n->is_null() )
      return;
    const auto next = n->find(*it);
    if( next == n->end() )
      return;
    n = &*next;
  }
  if( n->is_object() )
    n->erase(finalNode);
}


void trimAbsolute(nlohmann::json& json, PathDecomposition const& pd)
{
  BUT_ASSERT( pd.begin_ != pd.end_ );
  trimFrom(json, pd.begin_+1, pd.end_, *pd.finalNode_);
}

template<typename It>
void trimRelative(nlohmann::json& json, It beginPath, It endPath, std::string const& finalNode)
{
  if( json.is_null() )
    return;
  trimFrom(json, beginPath, endPath, finalNode);
  if( not json.is_object() && not json.is_array() )
    return;
  for(auto& e: json)
    trimRelative(e, beginPath, endPath, finalNode);
}

void trimRelative(nlohmann::json& json, PathDecomposition const& pd)
{
  trimRelative(json, pd.begin_, pd.end_, *pd.finalNode_);
}

void trimOneField(nlohmann::json& json, Path const& path)
{
  if( path.empty() )
    return;
  const auto pd = PathDecomposition{path};
  if( path.absolute() )
    trimAbsolute(json, pd);
  else
    trimRelative(json, pd);
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
