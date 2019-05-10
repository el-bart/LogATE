#pragma once
#include "CursATE/Curses/Window.hpp"
#include "CursATE/Curses/detail/ScrolableWindowBackend.hpp"

namespace CursATE::Curses
{

struct ScrolableWindow
{
  ScrolableWindow(DataSourceShNN dataSource, const ScreenPosition sp, const ScreenSize ss, const Window::Boxed boxed):
    backend_{std::move(dataSource)},
    window_{sp, ss, boxed},
    userAreaSize_{ window_.userAreaSize() }
  {
    backend_.resize( window_.userAreaSize() );
  }

  void refresh();

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

  void select(const DataSource::Id id) { backend_.select(id); }
  void selectNearest(const DataSource::Id id) { backend_.selectNearest(id); }
  But::Optional<DataSource::Id> currentSelection() const { return backend_.currentSelection(); }

private:
  struct DisplayDataSummary
  {
    auto operator==(DisplayDataSummary const& other) const
    {
      return first_   == other.first_ &&
             last_    == other.last_ &&
             size_    == other.size_ &&
             current_ == other.current_;
    }

    DataSource::Id first_{0};
    DataSource::Id last_{0};
    size_t size_{0};
    DataSource::Id current_{0};
  };

  detail::ScrolableWindowBackend backend_;
  Window window_;
  DisplayDataSummary dds_;
  ScreenSize userAreaSize_;
};

}
