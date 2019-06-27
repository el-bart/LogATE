#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/Curses/Window.hpp"
#include "CursATE/Curses/getChar.hpp"

using namespace CursATE::Curses;

namespace
{
std::string name(const int ch)
{
  const auto ptr = keyname(ch);
  if(not ptr)
    return "<nullptr>";
  return ptr;
}
}

int main()
{
  const Init init;
  set(CursorVisibility::Invisible);
  Window win{ ScreenPosition{Row{1}, Column{1}}, ScreenSize{Rows{24}, Columns{80}}, Window::Boxed::True };

  int ch = '?';
  for(auto i=0; ; ++i)
  {
    win.clear();
    mvwprintw( win.get(), 1, 1, "iteration %d: key_code=%d key_name=%s", i, ch, name(ch).c_str() );
    clrtoeol();

    win.refresh();
    const auto tmp = getChar(std::chrono::seconds{2});
    if(not tmp)
      continue;
    ch = *tmp;
    if(ch == 'q')
      break;
  }
}
