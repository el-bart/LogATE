#include "CursATE/Curses/detail/ScrolableWindowBackend.hpp"

namespace CursATE::Curses::detail
{

void ScrolableWindowBackend::update()
{
  const auto before = currentSelectionDistanceFromTheTop();
  auto after  = currentSelectionDistanceFromTheBottom();
  if( before + 1u + after < rows() )
    after = rows() -1u - before;
  const auto wasEmpty = buffer_.empty();
  const auto selected = currentSelection_ ? currentSelection_ : dataSource_->first();
  if(not selected)
    return;

  // TODO: no need to do this each and evey time...
  buffer_ = dataSource_->get(before, *selected, after);
  if( buffer_.empty() )
    return;
  if(wasEmpty)
    currentSelection_ = selected;

  // TODO: apply row offset here
  // TODO: apply column offset here

  (void)sideScrollOffset_;
  (void)upDownScrollOffset_;
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

ScrolableWindowBackend::DisplayData ScrolableWindowBackend::displayData() const
{
  DisplayData out;
  for(auto& e: buffer_)
    out.lines_.push_back(e);
  if(currentSelection_)
  {
    BUT_ASSERT( not buffer_.empty() );
    out.currentSelection_ = *currentSelection_;
  }
  return out;
}

bool ScrolableWindowBackend::ensureEnoughData(const size_t lines)
{
  (void)lines;
  return false;

/*
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
  */
}

bool ScrolableWindowBackend::loadEnoughData(const size_t lines)
{
  (void)lines;
  return false;
  /*
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
  */
}

size_t ScrolableWindowBackend::currentSelectionDistanceFromTheTop() const
{
  /*
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
  */
  return 0;
}

size_t ScrolableWindowBackend::currentSelectionDistanceFromTheBottom() const
{
  /*
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
  */
  return 0;
}

}
