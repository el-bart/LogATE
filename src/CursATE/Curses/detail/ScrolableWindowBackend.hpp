#pragma once
#include "CursATE/Curses/DataSource.hpp"
#include "CursATE/Curses/ScreenSize.hpp"
#include <But/Optional.hpp>
#include <string>
#include <vector>
#include <map>

namespace CursATE::Curses::detail
{

struct ScrolableWindowBackend
{
  explicit ScrolableWindowBackend(DataSourceShNN dataSource):
    dataSource_{std::move(dataSource)}
  { }

  void resize(ScreenSize ss) { ss_ = ss; }
  void update();

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
  DataSource::Id currentSelection() const { return *currentSelection_; }

  struct DisplayData final
  {
    std::vector<std::pair<DataSource::Id, std::string>> lines_;
    DataSource::Id currentSelection_;
  };

  DisplayData displayData() const;

private:
  auto rows() const { return static_cast<size_t>(ss_.rows_.value_); }
  size_t currentSelectionDistanceFromTheTop() const;
  size_t currentSelectionDistanceFromTheBottom() const;
  But::Optional<DataSource::Id> moveSelection(DataSource::Id now, int upDown) const;

  DataSourceShNN dataSource_;
  ScreenSize ss_{Rows{1}, Columns{1}};
  std::map<DataSource::Id, std::string> buffer_;
  But::Optional<DataSource::Id> currentSelection_;
  int sideScrollOffset_{0};
  int upDownScrollOffset_{0};
};

}