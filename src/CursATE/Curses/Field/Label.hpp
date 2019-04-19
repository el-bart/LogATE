#pragma once
#include "CursATE/Curses/FieldSize.hpp"
#include "CursATE/Curses/Window.hpp"
#include <string>

namespace CursATE::Curses::Field
{

struct Label
{
  std::string label_;
};

FieldSize size(Label const& b);
void draw(Window& win, ScreenPosition sp, FieldSize fs, Label const& b, bool selected);

}
