#pragma once
#include <vector>
#include <string>

namespace LogATE::Tree
{

struct Path final
{
  auto root() const { return value_.empty() || value_[0] == "."; }

  std::vector<std::string> value_;
};

}
