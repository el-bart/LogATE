#pragma once
#include "CursATE/Curses/DataSource.hpp"
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
  DataSource::Id currentSelection() const { return currentSelection_; }

  struct DisplayData
  {
    std::vector<std::string> lines_;
    size_t selectionIndex_;
  };

  DisplayData displayData(ScreenSize ss) const;

private:
  bool ensureEnoughData(size_t lines);
  bool loadEnoughData(size_t lines);
  size_t currentSelectionDistanceFromTheTop() const;

  DataSourceShNN dataSource_;
  std::map<DataSource::Id, std::string> buffer_;
  DataSource::Id currentSelection_;
  size_t sideScrollOffset_{0};
};

}
