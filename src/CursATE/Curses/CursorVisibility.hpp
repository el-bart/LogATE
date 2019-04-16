#pragma once
#include "CursATE/Curses/Exception.hpp"

namespace CursATE::Curses
{

enum class CursorVisibility
{
  Invisible = 0,
  Normal = 1,
  VeryVisible =2
};

BUT_DEFINE_EXCEPTION(FailedToSetCursorVisibility, Exception, "failed to set cursor visibility");

void set(const CursorVisibility cv);
CursorVisibility cursorVisibility();


struct CursorVisibilityGuard final
{
  explicit CursorVisibilityGuard(CursorVisibility newVisibility):
    previous_{ cursorVisibility() }
  {
    set(newVisibility);
  }

  ~CursorVisibilityGuard() { set(previous_); }

  CursorVisibilityGuard(CursorVisibilityGuard const&) = delete;
  CursorVisibilityGuard& operator=(CursorVisibilityGuard const&) = delete;

  CursorVisibilityGuard(CursorVisibilityGuard&&) = delete;
  CursorVisibilityGuard& operator=(CursorVisibilityGuard&&) = delete;

private:
  CursorVisibility previous_;
};

}
