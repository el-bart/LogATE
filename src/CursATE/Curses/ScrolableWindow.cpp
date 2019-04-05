#include "CursATE/Curses/ScrolableWindow.hpp"

namespace CursATE::Curses
{

void ScrolableWindow::refresh()
{
  const auto uap = window_.userAreaStartPosition();
  const auto uas = window_.userAreaSize();
  const auto displayData = backend_.displayData(uas);
  BUT_ASSERT( displayData.lines_.size() <= static_cast<size_t>(uas.rows_.value_) );

  for(auto i=0u; i<displayData.lines_.size(); ++i)
  {
    // TODO: add higlight for a selected item.
    mvwprintw( window_.get(), uap.row_.value_+i, uap.column_.value_, "%s", displayData.lines_[i].c_str() );
    clrtoeol();
  }

  window_.refresh();
}

}
