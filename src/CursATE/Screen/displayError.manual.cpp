#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/Screen/displayError.hpp"
#include <ncurses.h>

using namespace CursATE::Curses;
using namespace CursATE::Screen;

BUT_DEFINE_EXCEPTION(CustomError, But::Exception, "custom error");


int main()
{
  const Init init;
  set(CursorVisibility::Invisible);

  displayError({"line 1", "other line with more text"});
  clear();
  refresh();

  try
  {
    std::stringstream strm;
    for(auto i=99; i>0; --i)
      strm << i << " bottles on the wall... ";
    BUT_THROW( CustomError, "let's see how it looks like: " << strm.str() );
  }
  catch(std::exception const& ex)
  {
    displayError(ex);
  }
}
