#include "CursATE/Curses/ScrolableWindow.hpp"

namespace CursATE::Curses
{

void ScrolableWindow::refresh()
{
  /*
  const auto displayData = backend_.getStrings( window_.userAreaSize() );
  window_.refresh();

  //const auto uas = window_.userAreaSize();
  const auto uap = window_.userAreaStartPosition();

  const auto selectionIt = buffer_.find( currentSelection() );
  BUT_ASSERT( selectionIt != end(buffer_) );
  auto it = begin(buffer_);
  for(auto i=0u; i<lines; ++i)
  {
    auto& input = it->second;
    const auto from = input.begin() + std::min( input.size(), sideScrollOffset_ );
    const auto to   = from + std::min<size_t>( input.size() - sideScrollOffset_, uas.columns_.value_ );
    const auto str = std::string{from, to};
    mvwprintw( window_.get(), uap.row_.value_+i, uap.column_.value_, "%s", str.c_str() );
    ++it;
  }

  window_.refresh();
  */
}

}
