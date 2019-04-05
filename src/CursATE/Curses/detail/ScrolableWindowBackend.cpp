#include "CursATE/Curses/detail/ScrolableWindowBackend.hpp"

namespace CursATE::Curses::detail
{

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
  auto it = buffer_.find( currentSelection() );
  /*
  if( it == end(buffer_) )
    returnr
    */
  ++it;
  currentSelection_ = it->first;
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

ScrolableWindowBackend::DisplayData ScrolableWindowBackend::displayData(const ScreenSize ss)
{
  DisplayData out;
  loadEnoughData(ss.rows_.value_);
  BUT_ASSERT( buffer_.size() <= static_cast<size_t>(ss.rows_.value_) );
  if( buffer_.empty() )
    return out;
  const auto lines = std::min<size_t>( buffer_.size(), ss.rows_.value_ );

  const auto selectionIt = buffer_.find( currentSelection() );
  BUT_ASSERT( selectionIt != end(buffer_) );
  out.lines_.reserve(lines);
  auto it = begin(buffer_);
  for(auto i=0u; i<lines; ++i)
  {
    auto& input = it->second;
    const auto from = input.begin() + std::min( input.size(), sideScrollOffset_ );
    const auto to   = from + std::min<size_t>( input.size() - sideScrollOffset_, ss.columns_.value_ );
    out.lines_.emplace_back(it->first, std::string{from, to});
    ++it;
  }

  out.currentSelection_ = currentSelection_;

  return out;
}

void ScrolableWindowBackend::loadEnoughData(const size_t lines)
{
  if(lines == 0)
  {
    buffer_.clear();
    currentSelection_ = DataSource::Id{};
    return;
  }

  if( buffer_.empty() )
  {
    if( dataSource_->size() == 0 )
      return;
    currentSelection_ = dataSource_->first();
    auto opt = dataSource_->get(currentSelection_);
    if(not opt)
      return;
    buffer_.insert(*opt);
  }
  BUT_ASSERT( not buffer_.empty() );
  BUT_ASSERT( buffer_.size() <= lines );

  auto lastId = currentSelection_;
  while( buffer_.size() < lines )
  {
    auto opt = dataSource_->next(lastId);
    if(not opt)
      return;
    lastId = opt->first;
    buffer_.insert( std::move(*opt) );
  }

  BUT_ASSERT( buffer_.size() <= lines );
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

}
