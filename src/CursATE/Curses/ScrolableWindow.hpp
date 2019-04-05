#pragma once
#include "CursATE/Curses/Window.hpp"
#include "CursATE/Curses/detail/ScrolableWindowBackend.hpp"

namespace CursATE::Curses
{

struct ScrolableWindow
{
  ScrolableWindow(DataSourceShNN dataSource, const ScreenPosition sp, const ScreenSize ss, const Window::Boxed boxed):
    backend_{std::move(dataSource)},
    window_{sp, ss, boxed}
  { }

  void refresh();

  void scrollUp() { backend_.scrollUp(); }
  void scrollDown() { backend_.scrollDown(); }
  void scrollPageUp() { backend_.scrollPageUp(); }
  void scrollPageDown() { backend_.scrollPageDown(); }
  void scrollToListBegin() { backend_.scrollToListBegin(); }
  void scrollToListEnd() { backend_.scrollToListEnd(); }

  void scrollLeft() { backend_.scrollLeft(); }
  void scrollRight() { backend_.scrollRight(); }
  void scrollToLineBegin() { backend_.scrollToLineBegin(); }
  void scrollToLineEnd() { backend_.scrollToLineEnd(); }

  void selectUp() { backend_.selectUp(); }
  void selectDown() { backend_.selectDown(); }
  void selectPageUp() { backend_.selectPageUp(); }
  void selectPageDown() { backend_.selectPageDown(); }
  void selectFirst() { backend_.selectFirst(); }
  void selectLast() { backend_.selectLast(); }

  DataSource::Id currentSelection() const { return backend_.currentSelection(); }

private:
  detail::ScrolableWindowBackend backend_;
  Window window_;
};

}
