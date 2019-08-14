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
    case KEY_END: return 530;
    case 'l': return 12;
    case 'o': return 15;
    case 'e': return 5;
    case KEY_DC: return 519;
    case KEY_BACKSPACE: return 8;
  }
  // workaround for a GCC bug...
#if GCC_VERSION < 90000
  //BUT_ASSERT(!"unknown key mapping requested");
  return 0;
#else
  throw std::logic_error{"unknown key mapping requested"};
#endif
}

static constexpr int escapeKey = 27;

}
