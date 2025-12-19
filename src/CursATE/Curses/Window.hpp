#pragma once
#include "CursATE/Curses/ScreenPosition.hpp"
#include "CursATE/Curses/ScreenSize.hpp"
#include "CursATE/Curses/Exception.hpp"
#include "CursATE/Curses/Color.hpp"
#include <ncurses.h>
#include <optional>

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

  struct Captions final
  {
    std::optional<std::string> title_;
    std::optional<std::string> bottomCaption_;
  };

  Window(const ScreenPosition sp, const ScreenSize ss, const Boxed boxed):
    Window{sp, ss, boxed, Captions{}}
  { }

  Window(const ScreenPosition sp, const ScreenSize ss, Captions captions):
    Window{sp, ss, Boxed::True, std::move(captions)}
  { }

  ~Window()
  {
    if(boxed_)
      wborder(window_, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    clear();
    wrefresh(window_);
    delwin(window_);
  }

  Window(Window const&) = delete;
  Window& operator=(Window const&) = delete;

  Window(Window&&) = delete;
  Window& operator=(Window&&) = delete;

  void clear() const
  {
    wclear(window_);
  }

  void refresh() const
  {
    if(boxed_)
      box(window_, 0, 0);
    addCaptions();
    wrefresh(window_);
  }

  auto size() const { return ScreenSize{window_}; }
  auto position() const { return ScreenPosition{window_}; }
  auto boxed() const { return boxed_; }
  auto get() const { return window_; }

  auto userAreaStartPosition() const
  {
    if( boxed() )
      return ScreenPosition{ Row{1}, Column{1} };
    return ScreenPosition{ Row{0}, Column{0} };
  }
  auto userAreaSize() const
  {
    const auto ss = size();
    if( boxed() )
      return ScreenSize{ Rows{std::max(ss.rows_.value_-2, 0)}, Columns{std::max(ss.columns_.value_-2, 0)} };
    return ss;
  }

  void colors(const Color background, const Color font)
  {
    const auto cp = 1;
    init_pair( cp, static_cast<int>(font), static_cast<int>(background) );
    wbkgd(window_, COLOR_PAIR(cp));
  }

private:
  Window(const ScreenPosition sp, const ScreenSize ss, const Boxed boxed, Captions&& captions):
    boxed_{boxed == Boxed::True},
    captions_{std::move(captions)},
    window_{ newwin(ss.rows_.value_, ss.columns_.value_,
                    sp.row_.value_,  sp.column_.value_) }
  {
    if(not window_)
      BUT_THROW(FailedToCreateWindow, "nullptr received");
    keypad(window_, TRUE);   // enable funciton keys, arrows, etc...
  }

  void addCaptions() const
  {
    auto const s = size();
    if(captions_.title_)
    {
      auto const len = int( captions_.title_->size() );
      auto const r = 0;
      auto const c = std::max(0, (s.columns_.value_ - len)/2);
      if(c + len + 2/*corners*/ + 2/*spaces*/ <= s.columns_.value_)
        mvwprintw(window_, r, c, " %s ", captions_.title_->c_str());
    }
    if(captions_.bottomCaption_)
    {
      auto const len = int( captions_.bottomCaption_->size() );
      auto const r = s.rows_.value_-1;
      auto const c = std::min(s.columns_.value_, 5);
      if(c + len + 2/*corners*/ + 2/*spaces*/ <= s.columns_.value_)
        mvwprintw(window_, r, c, " %s ", captions_.bottomCaption_->c_str());
    }
  }

  const bool boxed_;
  const Captions captions_;
  WINDOW* window_;
};

}
