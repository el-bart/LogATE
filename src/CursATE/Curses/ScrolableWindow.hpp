#pragma once
#include "CursATE/Curses/Window.hpp"
#include "LogATE/Log.hpp"
#include <But/NotNull.hpp>
#include <But/Mpl/FreeOperators.hpp>
#include <map>

namespace CursATE::Curses
{

struct ScrolableWindow
{
  struct DataSource
  {
    struct Id { size_t value_{0}; };

    virtual ~DataSource() = default;

    virtual size_t size() const = 0;
    virtual Id first() const = 0;
    virtual Id last() const = 0;
    virtual std::map<Id, std::string> get(size_t before, Id id, size_t after) const = 0;
  };
  using DataSourceShNN = But::NotNullShared<DataSource>;

  ScrolableWindow(DataSourceShNN ds, const ScreenPosition sp, const ScreenSize ss, const Window::Boxed boxed):
    ds_{std::move(ds)},
    window_{sp, ss, boxed}
  { }

  void refresh() const;

  void scrollUp();
  void scrollDown();
  void scrollPageUp();
  void scrollPageDown();
  void scrollToListBegin();
  void scrollToListEnd();

  void scrollLeft();
  void scrollRight();
  void scrollToLineBegin();
  void scrollToLineEnd();

  void selectUp();
  void selectDown();
  void selectPageUp();
  void selectPageDown();
  void selectFirst();
  void selectLast();
  DataSource::Id currentSelection() const;

private:
  DataSourceShNN ds_;
  Window window_;
};

BUT_MPL_FREE_OPERATORS_COMPARE(ScrolableWindow::DataSource::Id, .value_)

}
