#include "CursATE/Curses/ScrolableWindow.hpp"

namespace CursATE::Curses
{

void ScrolableWindow::refresh()
{
  window_.clear();

  const auto uas = window_.userAreaSize();
  backend_.resize(uas);
  const auto displayData = backend_.displayData();
  BUT_ASSERT( displayData.lines_.size() <= static_cast<size_t>(uas.rows_.value_) );

  const auto uap = window_.userAreaStartPosition();
  for(auto i=0u; i<displayData.lines_.size(); ++i)
  {
    const auto isSelected = displayData.lines_[i].first == displayData.currentSelection_;
    const auto markAttr = A_REVERSE;
    if(isSelected)
      wattr_on( window_.get(), markAttr, nullptr);

    mvwprintw( window_.get(), uap.row_.value_+i, uap.column_.value_, "%s", displayData.lines_[i].second.c_str() );

    if(isSelected)
      wattr_off( window_.get(), markAttr, nullptr);
  }

  window_.refresh();
}

}
