#pragma once
#include "CursATE/Curses/FieldSize.hpp"
#include "CursATE/Curses/Window.hpp"
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

FieldSize size(Radio const& r);
void draw(Window& win, ScreenPosition sp, FieldSize fs, Radio const& r, bool selected);

}
