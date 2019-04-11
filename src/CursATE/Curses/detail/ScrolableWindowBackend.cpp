#include "CursATE/Curses/detail/ScrolableWindowBackend.hpp"

namespace CursATE::Curses::detail
{

void ScrolableWindowBackend::update()
{
  const auto surround = rows() > 0 ? rows()-1u : 0u;

  if(not currentSelection_)
  {
    currentSelection_ = dataSource_->first();
    if(not currentSelection_)
      return;
    buffer_ = dataSource_->get(0, *currentSelection_, surround);
  }
  BUT_ASSERT(currentSelection_);
  BUT_ASSERT( not buffer_.empty() );
  if( buffer_.size() < dataSource_->size() )
  {
    const auto before = currentSelectionDistanceFromTheTop();
    const auto after = surround - before;
    buffer_ = dataSource_->get(before, *currentSelection_, after);
  }

  const auto selected = moveSelection(*currentSelection_, upDownScrollOffset_);
  if(not selected)
    return;
  const auto lastOffset = upDownScrollOffset_;
  upDownScrollOffset_ = 0;
  if( buffer_.find(*selected) != end(buffer_) )
  {
    currentSelection_ = selected;
    return;
  }

  if(lastOffset < 0)
    buffer_ = dataSource_->get(0, *selected, surround);
  if(lastOffset > 0)
    buffer_ = dataSource_->get(surround, *selected, 0);
  currentSelection_ = selected;

  // TODO: apply column offset here

  (void)sideScrollOffset_;
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
  --upDownScrollOffset_;
}

void ScrolableWindowBackend::selectDown()
{
  ++upDownScrollOffset_;
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
  BUT_ASSERT( buffer_.size() <= rows() );
  DisplayData out;
  for(auto& e: buffer_)
    out.lines_.push_back(e);
  if(currentSelection_)
  {
    BUT_ASSERT( not buffer_.empty() );
    out.currentSelection_ = *currentSelection_;
  }
  BUT_ASSERT( out.lines_.size() <= rows() );
  return out;
}


size_t ScrolableWindowBackend::currentSelectionDistanceFromTheTop() const
{
  if(not currentSelection_)
    return 0;
  auto it = buffer_.find(*currentSelection_);
  if( it == end(buffer_) )
    return 0;
  size_t pos = 0;
  while( it != begin(buffer_) )
  {
    ++pos;
    --it;
  }
  BUT_ASSERT( pos < rows() );
  return pos;
}

size_t ScrolableWindowBackend::currentSelectionDistanceFromTheBottom() const
{
  if(not currentSelection_)
    return 0;
  auto it = buffer_.find(*currentSelection_);
  if( it == end(buffer_) )
    return 0;
  ++it;
  size_t pos = 0;
  while( it != end(buffer_) )
  {
    ++pos;
    ++it;
  }
  BUT_ASSERT( pos < rows() );
  return pos;
}

But::Optional<DataSource::Id> ScrolableWindowBackend::moveSelection(const DataSource::Id now, const int upDown) const
{
  if(upDown == 0)
    return now;

  if(upDown > 0)
  {
    const auto tmp = dataSource_->get(0, now, upDown);
    if( tmp.empty() )
      return {};
    return tmp.rbegin()->first;
  }

  if(upDown < 0)
  {
    const auto tmp = dataSource_->get(-upDown, now, 0);
    if( tmp.empty() )
      return {};
    return tmp.begin()->first;
  }

  BUT_ASSERT(!"code never reaches here");
  throw std::logic_error{"code should never reach here"};
}

}
