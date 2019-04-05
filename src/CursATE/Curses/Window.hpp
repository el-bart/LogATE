#pragma once
#include <ncurses.h>
#include "CursATE/Curses/ScreenPosition.hpp"
#include "CursATE/Curses/ScreenSize.hpp"
#include "CursATE/Curses/Exception.hpp"

namespace CursATE::Curses
{

struct Window
{
  BUT_DEFINE_EXCEPTION(FailedToCreateWindow, Exception, "failed to create window");

  enum class Boxed
  {
    False,
    True
  };

  Window(const ScreenPosition sp, const ScreenSize ss, const Boxed boxed):
    boxed_{boxed},
    window_{ newwin(ss.rows_.value_, ss.columns_.value_,
                    sp.row_.value_,  sp.column_.value_) }
  {
    if(not window_)
      BUT_THROW(FailedToCreateWindow, "nullptr received");
    keypad(window_, TRUE);   // enable funciton keys, arrows, etc...
  }

  ~Window()
  {
    switch(boxed_)
    {
      case Boxed::True: wborder(window_, ' ', ' ', ' ',' ',' ',' ',' ',' '); break;
      case Boxed::False: break;
    }
    wrefresh(window_);
    delwin(window_);
  }

  Window(Window const&) = delete;
  Window& operator=(Window const&) = delete;

  Window(Window&&) = delete;
  Window& operator=(Window&&) = delete;

  void refresh() const
  {
    switch(boxed_)
    {
      case Boxed::True: box(window_, 0, 0); break;
      case Boxed::False: break;
    }
    wrefresh(window_);
  }

  auto size() const { return ScreenSize{window_}; }
  auto position() const { return ScreenPosition{window_}; }

  const Boxed boxed_;
  WINDOW* window_;
};

}
