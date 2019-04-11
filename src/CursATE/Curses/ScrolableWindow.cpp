#include "CursATE/Curses/ScrolableWindow.hpp"

namespace CursATE::Curses
{

void ScrolableWindow::refresh()
{
  const auto uas = window_.userAreaSize();
  backend_.resize(uas);
  //backend_.update();
  const auto displayData = backend_.displayData();
  BUT_ASSERT( displayData.lines_.size() <= static_cast<size_t>(uas.rows_.value_) );

  const auto uap = window_.userAreaStartPosition();
  for(auto i=0u; i<displayData.lines_.size(); ++i)
  {
    if( displayData.lines_[i].first == displayData.currentSelection_ )
    {
      // TODO: add higlight for a selected item.
    }
    mvwprintw( window_.get(), uap.row_.value_+i, uap.column_.value_, "%s", displayData.lines_[i].second.c_str() );
    clrtoeol();
  }

  window_.refresh();
}

}
