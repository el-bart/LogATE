#include "CursATE/Curses/detail/ScrolableWindowBackend.hpp"

namespace CursATE::Curses::detail
{

/*
void ScrolableWindowBackend::scrollUp()
{
  //const auto ss = window_.size();
  //const auto sel = currentSelection();
  //buffer_ = loadData(ss, sel);
  // TODO
}

void ScrolableWindowBackend::scrollDown()
{
  // TODO
}

void ScrolableWindowBackend::scrollPageUp()
{
  // TODO
}

void ScrolableWindowBackend::scrollPageDown()
{
  // TODO
}

void ScrolableWindowBackend::scrollToListBegin()
{
  // TODO
}

void ScrolableWindowBackend::scrollToListEnd()
{
  // TODO
}


void ScrolableWindowBackend::scrollLeft()
{
  // TODO
}

void ScrolableWindowBackend::scrollRight()
{
  // TODO
}

void ScrolableWindowBackend::scrollToLineBegin()
{
  // TODO
}

void ScrolableWindowBackend::scrollToLineEnd()
{
  // TODO
}


void ScrolableWindowBackend::selectUp()
{
  // TODO
}

void ScrolableWindowBackend::selectDown()
{
  // TODO
}

void ScrolableWindowBackend::selectPageUp()
{
  // TODO
}

void ScrolableWindowBackend::selectPageDown()
{
  // TODO
}

void ScrolableWindowBackend::selectFirst()
{
  // TODO
}

void ScrolableWindowBackend::selectLast()
{
  // TODO
}

DisplayData ScrolableWindowBackendBackend::displayData(const ScreenSize ss) const;
{
  ensureEnoughData(ss.rows_.value_);
  if( buffer_.empty() )
  {
    window_.refresh();
    return;
  }
  const auto uas = window_.userAreaSize();
  const auto uap = window_.userAreaStartPosition();
  BUT_ASSERT( buffer_.size() <= static_cast<size_t>(uas.rows_.value_) );
  const auto lines = std::min<size_t>( buffer_.size(), uas.rows_.value_ );

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
}

bool ScrolableWindowBackend::ensureEnoughData(const size_t lines)
{
  if( buffer_.size() == lines )
  {
    // TODO
    if( buffer_.size() == dataSource_->size() )
      return false;

  }

  if( buffer_.size() < rows )
  {
    if( buffer_.size() != dataSource_->size() )
      return loadEnoughData();
    return false
  }
  return buffer_.size() == rows;
}

bool ScrolableWindowBackend::loadEnoughData(const size_t lines)
{
  const auto needUpTo = static_cast<size_t>(window_.userAreaSize().rows_.value_);

  if( buffer_.empty() )
  {
    if( dataSource_->size() == 0 )
      return false;
    currentSelection_ = dataSource_->first();
    buffer_ = dataSource_->get(0, currentSelection(), std::max<size_t>(needUpTo-1, 0));
    return true;
  }

  const auto needSurrounding = std::max<size_t>(needUpTo-1, 0);
  const auto needBefore = std::min<size_t>( currentSelectionDistanceFromTheTop(), needSurrounding );
  BUT_ASSERT( needSurrounding >= needBefore );
  const auto needAfter = needSurrounding - needBefore;
  buffer_ = dataSource_->get(needBefore, currentSelection(), needAfter);
  return true;
}

size_t ScrolableWindowBackend::currentSelectionDistanceFromTheTop() const
{
  auto it = buffer_.find( currentSelection() );
  if( it == end(buffer_) )
    return 0;

  size_t pos = 1;
  while( it != begin(buffer_) )
  {
    ++pos;
    --it;
  }
  return pos;
}
*/


}
