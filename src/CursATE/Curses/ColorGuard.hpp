#pragma once
#include "CursATE/Curses/Color.hpp"
#include "CursATE/Curses/Window.hpp"

namespace CursATE::Curses
{

struct ColorGuard final
{
  ColorGuard(Window& win, Color font, Color background);
  ~ColorGuard();

  ColorGuard(ColorGuard const&) = delete;
  ColorGuard& operator=(ColorGuard const&) = delete;

  ColorGuard(ColorGuard&&) = delete;
  ColorGuard& operator=(ColorGuard&&) = delete;

  void clear();

  auto fontColor() const { return font_; }
  auto backgroundColor() const { return background_; }

private:
  short colorPairId() const;

  Window& win_;
  const Color font_;
  const Color background_;
  bool cleared_{false};
};

}
