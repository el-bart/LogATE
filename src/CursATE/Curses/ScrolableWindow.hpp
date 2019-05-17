#pragma once
#include "CursATE/Curses/Window.hpp"
#include "CursATE/Curses/detail/ScrolableWindowBackend.hpp"
#include "CursATE/Curses/Exception.hpp"
#include <functional>
#include <string>

namespace CursATE::Curses
{

struct ScrolableWindow
{
  ScrolableWindow(DataSourceShNN dataSource,
                  const ScreenPosition sp,
                  const ScreenSize ss,
                  const Window::Boxed boxed,
                  std::function<std::string(size_t)> status = {}):
    dataSource_{std::move(dataSource)},
    backend_{dataSource_},
    window_{sp, ss, boxed},
    userAreaSize_{ window_.userAreaSize() },
    status_{ std::move(status) }
  {
    backend_.resize( window_.userAreaSize() );
  }

  void refresh();
  void forceNextRefresh() { contentChanged_ = true; }

  void scrollLeft()
  {
    for(auto i=0u; i<leftRightScrollMultiplier_; ++i)
      backend_.scrollLeft();
    contentChanged_ = true;
  }
  void scrollRight()
  {
    for(auto i=0u; i<leftRightScrollMultiplier_; ++i)
      backend_.scrollRight();
    contentChanged_ = true;
  }
  void scrollToLineBegin() { backend_.scrollToLineBegin(); contentChanged_ = true; }
  void scrollToLineEnd() { backend_.scrollToLineEnd(); contentChanged_ = true; }

  void selectFirstVisible() { backend_.selectFirstVisible(); contentChanged_ = true; }
  void selectMiddleVisible() { backend_.selectMiddleVisible(); contentChanged_ = true; }
  void selectLastVisible() { backend_.selectLastVisible(); contentChanged_ = true; }

  void selectUp() { backend_.selectUp(); contentChanged_ = true; }
  void selectDown() { backend_.selectDown(); contentChanged_ = true; }
  void selectPageUp() { backend_.selectPageUp(); contentChanged_ = true; }
  void selectPageDown() { backend_.selectPageDown(); contentChanged_ = true; }
  void selectFirst() { backend_.selectFirst(); contentChanged_ = true; }
  void selectLast() { backend_.selectLast(); contentChanged_ = true; }

  void select(const DataSource::Id id) { backend_.select(id); contentChanged_ = true; }
  void selectNearest(const DataSource::Id id) { backend_.selectNearest(id); contentChanged_ = true; }
  But::Optional<DataSource::Id> currentSelection() const { return backend_.currentSelection(); }

private:
  void displayStatus();
  ScreenSize userAreaSize() const;

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

  const unsigned leftRightScrollMultiplier_{10};
  bool contentChanged_{false};
  DataSourceShNN dataSource_;
  detail::ScrolableWindowBackend backend_;
  Window window_;
  DisplayDataSummary dds_;
  ScreenSize userAreaSize_;
  std::function<std::string(size_t)> status_;
};

}
