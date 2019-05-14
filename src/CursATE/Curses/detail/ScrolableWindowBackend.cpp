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
  trimBufferToFitNewSize();
  update();
}


void ScrolableWindowBackend::update()
{
  offsetBy(0);
}


void ScrolableWindowBackend::scrollLeft()
{
  if( sideScrollOffset_ == 0 )
    return;
  --sideScrollOffset_;
}

void ScrolableWindowBackend::scrollRight()
{
  const auto maxSize = longestStringInBuffer();
  const auto columns = static_cast<size_t>(ss_.columns_.value_);
  if( sideScrollOffset_ + columns >= maxSize )
    return;
  ++sideScrollOffset_;
}

void ScrolableWindowBackend::scrollToLineBegin()
{
  sideScrollOffset_ = 0;
}

void ScrolableWindowBackend::scrollToLineEnd()
{
  const auto maxSize = longestStringInBuffer();
  const auto columns = static_cast<size_t>(ss_.columns_.value_);
  if( maxSize <= columns )
    sideScrollOffset_ = 0;
  else
    sideScrollOffset_ = maxSize - columns;
}


void ScrolableWindowBackend::selectFirstVisible()
{
  if( buffer_.empty() )
    return;
  currentSelection_ = buffer_.begin()->first;
}


void ScrolableWindowBackend::selectMiddleVisible()
{
  if( buffer_.empty() )
    return;
  auto it = begin(buffer_);
  std::advance( it, buffer_.size()/2 );
  BUT_ASSERT( it != end(buffer_) );
  currentSelection_ = it->first;
}


void ScrolableWindowBackend::selectLastVisible()
{
  if( buffer_.empty() )
    return;
  currentSelection_ = buffer_.rbegin()->first;
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


void ScrolableWindowBackend::selectNearest(const DataSource::Id id)
{
  const auto nearest = dataSource_->nearestTo(id);
  if(not nearest)
    return;
  select(*nearest);
}


namespace
{
template<typename C>
auto countBeforeAfter(C const& c, const DataSource::Id id)
{
  auto countBefore = 0u;
  auto countAfter = 0u;
  auto isBefore = true;
  for(auto& e: c)
  {
    if( e.first == id )
    {
      isBefore = false;
      continue;
    }
    if(isBefore)
      ++countBefore;
    else
      ++countAfter;
  }
  return std::make_pair(countBefore, countAfter);
}

template<typename C>
void eraseToEqualSize(C& c, const size_t surround, size_t countBefore, size_t countAfter)
{
  do
  {
    if( countBefore >= countAfter )
    {
      c.erase( c.begin() );
      --countBefore;
    }
    else
    {
      c.erase( c.rbegin()->first );
      --countAfter;
    }
  }
  while( c.size() > surround + 1u );
}
}

void ScrolableWindowBackend::select(const DataSource::Id id)
{
  if(currentSelection_ && *currentSelection_ == id)
    return;

  const auto surround = this->surround();
  buffer_ = dataSource_->get(surround, id, surround);

  if( buffer_.size() > surround + 1u )
  {
    auto [countBefore, countAfter] = countBeforeAfter(buffer_, id);
    eraseToEqualSize(buffer_, surround, countBefore, countAfter);
  }

  if( buffer_.empty() )
    currentSelection_.reset();
  else
    currentSelection_ = id;
}


ScrolableWindowBackend::DisplayData ScrolableWindowBackend::displayData() const
{
  DisplayData out;
  BUT_ASSERT( buffer_.size() <= rows() );
  for(auto& e: buffer_)
    out.lines_.push_back( std::make_pair( e.first, trimStringToFitOffset(e.second) ) );
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
  dropLeadingDeadElementsFromBuffer();

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


void ScrolableWindowBackend::trimBufferToFitNewSize()
{
  if( buffer_.size() <= rows() )
    return;
  if(not currentSelection_)
    return;
  trimFromEnd();
  trimFromBegin();
}


void ScrolableWindowBackend::trimFromEnd()
{
  BUT_ASSERT( not buffer_.empty() );
  while( buffer_.size() > rows() && buffer_.rbegin()->first != *currentSelection_)
  {
    buffer_.erase( buffer_.rbegin()->first );
    BUT_ASSERT( buffer_.find(*currentSelection_) != end(buffer_) );
  }
}


void ScrolableWindowBackend::trimFromBegin()
{
  BUT_ASSERT( not buffer_.empty() );
  while( buffer_.size() > rows() && buffer_.begin()->first != *currentSelection_)
  {
    buffer_.erase( buffer_.begin() );
    BUT_ASSERT( buffer_.find(*currentSelection_) != end(buffer_) );
  }
}


bool ScrolableWindowBackend::existsInDataSet(DataSource::Id id) const
{
  return not dataSource_->get(0, id, 0).empty();
}


void ScrolableWindowBackend::dropLeadingDeadElementsFromBuffer()
{
  while( not buffer_.empty() && not existsInDataSet( buffer_.begin()->first ) )
    buffer_.erase( buffer_.begin() );
  if(not currentSelection_)
    return;
  if( buffer_.find(*currentSelection_) != end(buffer_) )
    return;
  if( buffer_.empty() )
    currentSelection_.reset();
  else
    currentSelection_ = buffer_.begin()->first;
}


size_t ScrolableWindowBackend::longestStringInBuffer() const
{
  size_t maxLen = 0u;
  for(auto& e: buffer_)
    if( e.second.size() > maxLen )
      maxLen = e.second.size();
  return maxLen;
}


std::string ScrolableWindowBackend::trimStringToFitOffset(std::string const& in) const
{
  if( in.size() <= sideScrollOffset_ )
    return "";
  const auto from = begin(in) + sideScrollOffset_;
  const auto columns = static_cast<size_t>(ss_.columns_.value_);
  if( in.size() < sideScrollOffset_ + columns )
    return std::string{ from, end(in) };
  return std::string{ from, from + columns };
}

}
