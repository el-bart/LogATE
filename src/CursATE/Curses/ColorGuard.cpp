#include "CursATE/Curses/ColorGuard.hpp"
#include <ncurses.h>
#include <unordered_map>

namespace CursATE::Curses
{

namespace
{
struct ColorPair
{
  Color font_;
  Color background_;
};

auto operator==(const ColorPair lhs, const ColorPair rhs)
{
  return lhs.font_       == rhs.font_ &&
         lhs.background_ == rhs.background_;
}


struct ColorPairHasher
{
  auto operator()(const ColorPair cp) const
  {
    return hash_(static_cast<int>(cp.font_)) ^ hash_(static_cast<int>(cp.background_));
  }

  const std::hash<int> hash_{};
};
}


ColorGuard::ColorGuard(Window& win, const Color font, const Color background):
  win_{win},
  font_{font},
  background_{background}
{
  const auto cp = colorPairId();
  wattron( win_.get(), COLOR_PAIR(cp) );
  wbkgd( win_.get(), COLOR_PAIR(cp) );
}

ColorGuard::~ColorGuard()
{
  clear();
}


void ColorGuard::clear()
{
  if(cleared_)
    return;
  wattroff( win_.get(), COLOR_PAIR(colorPairId()) );
  cleared_ = true;
}


short ColorGuard::colorPairId() const
{
  static auto nextFreeId = 1;
  static std::unordered_map<ColorPair, int, ColorPairHasher> cp2id;

  const auto cp = ColorPair{font_, background_};

  {
    const auto it = cp2id.find(cp);
    if(it != end(cp2id))
      return it->second;
  }

  const auto id = nextFreeId++;
  init_pair( id, static_cast<int>(font_), static_cast<int>(background_) );
  cp2id[cp] = id;
  return id;
}

}
