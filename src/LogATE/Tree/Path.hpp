#pragma once
#include <vector>
#include <string>

namespace LogATE::Tree
{

struct Path final
{
  auto root() const { return value_.empty() || value_[0] == "."; }
  auto begin() const { return value_.begin(); }
  auto end() const { return value_.end(); }

  std::vector<std::string> value_;
};

}
