#include "CursATE/Curses/Field/Radio.hpp"
#include "CursATE/Curses/Field/detail/resizePadded.hpp"

namespace CursATE::Curses::Field
{

FieldSize size(Radio const& r)
{
  auto valuesSize = 0u;
  for(auto& e: r.values_)
    valuesSize += 1 + 3 + e.size();
  valuesSize += r.values_.size() - 1;   // spacing between elements
  return FieldSize{ r.label_.size(), valuesSize };
}

void draw(Window& win, const ScreenPosition sp, const FieldSize fs, Radio const& r, const bool selected)
{
  const auto markAttr = A_REVERSE;

  mvwprintw(win.get(), sp.row_.value_, sp.column_.value_, "%s", detail::resizePadded(r.label_, fs.label_, 0).c_str() );

  for(auto i=0u; i<r.values_.size(); ++i)
  {
    wprintw(win.get(), " ");
    auto mark = "[ ]";
    if( i == r.selection_ )
    {
      mark = "[X]";
      if(selected)
        wattr_on( win.get(), markAttr, nullptr);
    }
    wprintw(win.get(), "%s", mark);
    if( i == r.selection_ )
      if(selected)
        wattr_off( win.get(), markAttr, nullptr);
    wprintw(win.get(), " %s", r.values_[i].c_str());
  }
}

}
