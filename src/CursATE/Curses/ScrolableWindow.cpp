#include "CursATE/Curses/ScrolableWindow.hpp"

namespace CursATE::Curses
{

namespace
{
template<typename DDS>
auto displayDataSummary(detail::ScrolableWindowBackend::DisplayData const& dd)
{
  DDS dds;
  if( dd.lines_.empty() )
    return dds;
  dds.first_ = dd.lines_.front().first;
  dds.last_ = dd.lines_.back().first;
  dds.size_ = dd.lines_.size();
  dds.current_ = dd.currentSelection_;
  return dds;
}
}

void ScrolableWindow::refresh()
{
  const auto uas = window_.userAreaSize();
  backend_.resize(uas);
  const auto displayData = backend_.displayData();
  BUT_ASSERT( displayData.lines_.size() <= static_cast<size_t>(uas.rows_.value_) );
  const auto dds = displayDataSummary<DisplayDataSummary>(displayData);
  if( userAreaSize_ == uas && dds_ == dds )
    return;

  window_.clear();

  const auto uap = window_.userAreaStartPosition();
  for(auto i=0u; i<displayData.lines_.size(); ++i)
  {
    const auto isSelected = displayData.lines_[i].first == displayData.currentSelection_;
    const auto markAttr = A_REVERSE;
    if(isSelected)
      wattr_on( window_.get(), markAttr, nullptr);

    mvwprintw( window_.get(), uap.row_.value_+i, uap.column_.value_, "%s", displayData.lines_[i].second.c_str() );
    for(auto j = displayData.lines_[i].second.size(); j < static_cast<size_t>(uas.columns_.value_); ++j)
      wprintw( window_.get(), " ");

    if(isSelected)
      wattr_off( window_.get(), markAttr, nullptr);
  }

  window_.refresh();

  userAreaSize_ = uas;
  dds_ = dds;
}

}
