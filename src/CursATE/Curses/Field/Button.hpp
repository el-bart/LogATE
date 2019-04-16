#pragma once
#include "CursATE/Curses/FieldSize.hpp"
#include "CursATE/Curses/Window.hpp"
#include <string>

namespace CursATE::Curses::Field
{

struct Button
{
  std::string label_;
  bool clicked_{false};
};

FieldSize size(Button const& b);
void draw(Window& win, ScreenPosition sp, FieldSize fs, Button const& b, bool selected);

}
