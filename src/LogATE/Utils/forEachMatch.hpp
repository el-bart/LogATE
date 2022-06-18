#pragma once
#include "LogATE/Tree/Path.hpp"
#include <nlohmann/json.hpp>

namespace LogATE::Utils
{

template<typename F>
bool forEachMatch(nlohmann::json const& node, Tree::Path const& path, F&& f)
{
  (void)node;
  (void)path;
  (void)f;
  return false;
}

}
