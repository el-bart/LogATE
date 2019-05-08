#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/Screen/displayError.hpp"

using namespace CursATE::Curses;
using namespace CursATE::Screen;


int main()
{
  const Init init;
  set(CursorVisibility::Invisible);
  displayError({"line 1", "other line with more text"});
}
