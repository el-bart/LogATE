#include "CursATE/Curses/detail/ScrolableWindowBackend.hpp"

namespace CursATE::Curses::detail
{

namespace
{
auto windowTooSmall(const ScreenSize ss) { return ss.rows_.value_ == 0 || ss.columns_.value_ == 0; }
}

void ScrolableWindowBackend::resize(const ScreenSize in)
{
  if( windowTooSmall(in) )
    BUT_THROW(WindowTooSmall, "got: " << in.rows_.value_ << 'x' << in.columns_.value_);
  ss_ = in;
  update();
}

void ScrolableWindowBackend::update()
{
  offsetBy(0);
}

void ScrolableWindowBackend::scrollLeft()
{
  (void)sideScrollOffset_;
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
  offsetBy(-1);
}

void ScrolableWindowBackend::selectDown()
{
  offsetBy(+1);
}

void ScrolableWindowBackend::selectPageUp()
{
  offsetBy( -rows() );
}

void ScrolableWindowBackend::selectPageDown()
{
  offsetBy( +rows() );
}

void ScrolableWindowBackend::selectFirst()
{
  currentSelection_ = dataSource_->first();
  if(not currentSelection_)
    return;
  buffer_ = dataSource_->get( 0, *currentSelection_, surround() );
}

void ScrolableWindowBackend::selectLast()
{
  currentSelection_ = dataSource_->last();
  if(not currentSelection_)
    return;
  buffer_ = dataSource_->get( surround(), *currentSelection_, 0 );
}

ScrolableWindowBackend::DisplayData ScrolableWindowBackend::displayData() const
{
  DisplayData out;
  BUT_ASSERT( buffer_.size() <= rows() );
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


void ScrolableWindowBackend::offsetBy(const int offset)
{
  const auto surround = this->surround();

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

  if(offset == 0)
    return;

  const auto selected = moveSelection(*currentSelection_, offset);
  if(not selected)
    return;
  if( buffer_.find(*selected) != end(buffer_) )
  {
    currentSelection_ = selected;
    return;
  }

  if(offset < 0)
    buffer_ = dataSource_->get(0, *selected, surround);
  if(offset > 0)
    buffer_ = dataSource_->get(surround, *selected, 0);
  currentSelection_ = selected;
}

}
