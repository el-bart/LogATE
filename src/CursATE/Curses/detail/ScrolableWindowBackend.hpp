#pragma once
#include "CursATE/Curses/DataSource.hpp"
#include "CursATE/Curses/ScreenSize.hpp"
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

  void refresh();

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
  DataSource::Id currentSelection() const { return currentSelection_; }

  struct DisplayData final
  {
    std::vector<std::pair<DataSource::Id, std::string>> lines_;
    DataSource::Id currentSelection_;
  };

  DisplayData displayData(ScreenSize ss);

private:
  bool ensureEnoughData(size_t lines);
  bool loadEnoughData(size_t lines);
  size_t currentSelectionDistanceFromTheTop() const;

  DataSourceShNN dataSource_;
  std::map<DataSource::Id, std::string> buffer_;
  DataSource::Id currentSelection_;
  size_t sideScrollOffset_{0};
  size_t upDownScrollOffset_{0};
};

}
