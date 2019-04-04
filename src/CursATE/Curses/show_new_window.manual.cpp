#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/Window.hpp"

using namespace CursATE::Curses;

int main()
{
  const Init init;
  Window win{ ScreenPosition{Row{2}, Column{10}}, ScreenSize{Rows{5}, Columns{20}}, Window::Boxed::True };
  win.refresh();
  getch();
}
