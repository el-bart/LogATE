#include "CursATE/Curses/Field/Radio.hpp"

namespace CursATE::Curses::Field
{

FieldSize size(Radio const& r)
{
  auto valuesSize = 0u;
  for(auto& e: r.values_)
    valuesSize += 4 + e.size();
  return FieldSize{ r.label_.size(), valuesSize };
}

void draw(Window& win, const ScreenPosition sp, const FieldSize fs, Radio const& r)
{
  (void)win;
  (void)sp;
  (void)fs;
  (void)r;
  // TODO
}

}
