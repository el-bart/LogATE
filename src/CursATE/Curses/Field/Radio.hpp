#pragma once
#include <string>
#include <vector>

namespace CursATE::Curses::Field
{

struct Radio
{
  std::string label_;
  std::vector<std::string> values_;
  unsigned selection_{0};
};

}
