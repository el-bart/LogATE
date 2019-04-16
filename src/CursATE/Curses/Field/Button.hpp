#pragma once
#include <string>

namespace CursATE::Curses::Field
{

struct Button
{
  std::string label_;
  bool clicked_{false};
};

}
