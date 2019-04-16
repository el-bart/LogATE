#pragma once
#include "CursATE/Curses/FieldSize.hpp"
#include "CursATE/Curses/Window.hpp"
#include <string>

namespace CursATE::Curses::Field
{

struct Input
{
  std::string label_;
  std::string value_{};
  unsigned cursorPosition_{0};
};

FieldSize size(Input const& i);
void draw(Window& win, ScreenPosition sp, FieldSize fs, Input const& i, bool selected);

}
