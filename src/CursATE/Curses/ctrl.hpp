#pragma once
#include <ncurses.h>

namespace CursATE::Curses
{

/** @brief return ctrl-<key> version of a key.
 *  @warning hack and will most likely not work on some terminals...
 */
inline constexpr auto ctrl(const int k)
{
  // DON'T ASK...
  switch(k)
  {
    case KEY_HOME: return 535;
    case KEY_END:  return 530;
  }
  throw std::logic_error{"unknown key mapping requested"};
}

}
