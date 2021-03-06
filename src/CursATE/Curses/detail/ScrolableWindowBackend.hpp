#pragma once
#include "CursATE/Curses/DataSource.hpp"
#include "CursATE/Curses/ScreenSize.hpp"
#include "CursATE/Curses/Exception.hpp"
#include <But/Optional.hpp>
#include <string>
#include <vector>
#include <map>

namespace CursATE::Curses::detail
{

struct ScrolableWindowBackend
{
  BUT_DEFINE_EXCEPTION(WindowTooSmall, Exception, "window is too small - minumum size is 1x1");

  explicit ScrolableWindowBackend(DataSourceShNN dataSource):
    dataSource_{std::move(dataSource)}
  { }

  void resize(ScreenSize ss);
  void update();

  void scrollLeft();
  void scrollRight();
  void scrollToLineBegin();
  void scrollToLineEnd();

  void selectFirstVisible();
  void selectMiddleVisible();
  void selectLastVisible();

  void selectUp();
  void selectDown();
  void selectPageUp();
  void selectPageDown();
  void selectFirst();
  void selectLast();
  But::Optional<DataSource::Id> currentSelection() const { return currentSelection_; }
  void select(DataSource::Id id);
  void selectNearest(DataSource::Id id);

  struct DisplayData final
  {
    std::vector<std::pair<DataSource::Id, std::string>> lines_;
    //                                    ^^^^^^^^^^^ TODO: can be std::string_view
    DataSource::Id currentSelection_;
  };

  DisplayData displayData() const;

private:
  using Buffer = std::map<DataSource::Id, std::string>;

  auto rows() const { return static_cast<size_t>(ss_.rows_.value_); }
  size_t currentSelectionDistanceFromTheTop() const;
  But::Optional<DataSource::Id> moveSelection(DataSource::Id now, int upDown) const;
  auto surround() const { return rows() > 0 ? rows()-1u : 0u; }
  void offsetBy(int offset);
  void trimBufferToFitNewSize();
  void trimFromEnd();
  void trimFromBegin();
  bool existsInDataSet(DataSource::Id id) const;
  void dropLeadingDeadElementsFromBuffer();
  size_t longestStringInBuffer() const;
  std::string trimStringToFitOffset(std::string const& in) const;

  DataSourceShNN dataSource_;
  ScreenSize ss_{Rows{1}, Columns{1}};
  Buffer buffer_;
  But::Optional<DataSource::Id> currentSelection_;
  unsigned sideScrollOffset_{0};
};

}
