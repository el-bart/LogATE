#include "CursATE/Curses/getChar.hpp"
#include <But/assert.hpp>
#include <ncurses.h>

namespace CursATE::Curses
{

But::Optional<int> getChar(const std::chrono::milliseconds tout)
{
  timeout( tout.count() );
  const auto ch = getch();
  timeout(-1);
  if(ch == ERR)
    return {};
  return ch;
}


int getChar()
{
  timeout(-1);
  const auto ch = getch();
  BUT_ASSERT(ch != ERR);
  return ch;
}

}
