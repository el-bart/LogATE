#pragma once
#include "CursATE/Curses/Change.hpp"
#include "CursATE/Curses/FieldSize.hpp"
#include "CursATE/Curses/Window.hpp"
#include "CursATE/Curses/Field/Input.hpp"
#include "CursATE/Curses/Field/Radio.hpp"
#include "CursATE/Curses/Field/Button.hpp"
#include "CursATE/Curses/Field/detail/resizePadded.hpp"
#include "CursATE/Curses/detail/TupleVisitor.hpp"
#include "CursATE/Curses/detail/TupleForEach.hpp"
#include <But/NotNull.hpp>
#include <tuple>

namespace CursATE::Curses
{

template<typename ...Fields>
struct Form final
{
  using Result = std::array<std::string, sizeof...(Fields)>;

  explicit Form(Fields&& ...fields):
    fields_{ std::forward<Fields>(fields)... },
    window_{ But::makeUniqueNN<Window>(ScreenPosition{Row{0}, Column{0}}, ScreenSize::global(), Window::Boxed::True) }
  { }

  constexpr static auto size() { return sizeof...(Fields); }

  Result process()
  {
    auto selected = 0;
    while(true)
    {
      drawAll(selected);
      const auto action = processElement(selected);
      switch(action)
      {
        case Change::Update: break;
        case Change::Next: selected = (selected+1) % size(); break;
        case Change::Previous: selected = (selected == 0) ? size()-1u : selected-1; break;
        case Change::Exit: return prepareResult();
      }
    }
  }

private:
  Result prepareResult()
  {
    Result out;
    auto n = 0;
    auto writer = [&](auto const& e) { out[n++] = this->getResult(e); };
    detail::TupleForEach<0, size()>::visit(fields_, writer);
    return out;
  }

  auto calculateSpacing() const
  {
    FieldSize fs;
    auto updateSize = [&](auto const& e)
        {
          const auto tmp = Field::size(e);
          fs.label_ = std::max(fs.label_, tmp.label_);
          fs.value_ = std::max(fs.value_, tmp.value_);
        };
    detail::TupleForEach<0, size()>::visit(fields_, updateSize);
    return fs;
  }

  void drawAll(const int selected)
  {
    window_->clear();
    const auto fs = calculateSpacing();
    auto uasp = window_->userAreaStartPosition();
    auto n = 0;
    auto draw = [&](auto const& e)
        {
          const auto nowSelected = selected==n;
          Field::draw(*window_, uasp, fs, e, nowSelected);
          ++uasp.row_.value_;
          ++n;
        };
    detail::TupleForEach<0, size()>::visit(fields_, draw);
    window_->refresh();
  }

  std::string getResult(Field::Button const& button) const { return button.clicked_ ? "true" : "false"; }
  std::string getResult(Field::Input const& input) const { return input.value_; }
  std::string getResult(Field::Radio const& radio) const
  {
    BUT_ASSERT( radio.selection_ < radio.values_.size() && "inde out of bound" );
    return radio.values_[radio.selection_];
  }

  Change processElement(int n)
  {
    auto row = 0u;
    auto processor = [&](auto& e) { return this->action(e, row++); };
    return detail::TupleVisitor<0, size()>::visit(n, fields_, processor);
  }

  Change action(Field::Button& button, const unsigned row)
  {
    (void)row;
    switch( getch() )
    {
      case KEY_UP: return Change::Previous;
      case KEY_DOWN: return Change::Next;
      case ' ':
      case 10:
      case KEY_ENTER: button.clicked_ = true; return Change::Exit;
      case 'q': return Change::Exit;
    }
    return Change::Update;
  }

  void positionCursorInInputField(Field::Input const& input, const unsigned row)
  {
    const auto fs = calculateSpacing();
    const auto uasp = window_->userAreaStartPosition();
    const auto vs = Field::detail::resizePaddedVisibleSize(input.value_, fs.value_, input.cursorPosition_);
    const auto valueStartPos = uasp.column_.value_ + fs.label_ + 1u + vs.selectionOffset_;
    wmove(window_->get(), uasp.row_.value_ + row, valueStartPos);
    window_->refresh();
  }

  Change action(Field::Input& input, const unsigned row)
  {
    positionCursorInInputField(input, row);
    const CursorVisibilityGuard cvg{CursorVisibility::Normal};
    switch( getch() )
    {
      case KEY_UP:
           return Change::Previous;
      case KEY_DOWN:
      case 10:
      case KEY_ENTER:
           return Change::Next;
      case KEY_RIGHT:
           input.cursorPosition_ = std::min<size_t>( input.cursorPosition_+1u, input.value_.size() );
           break;
      case KEY_LEFT:
           if( input.value_.empty() )
           {
             input.cursorPosition_ = 0;
             break;
           }
           if( input.cursorPosition_ == 0 )
             break;
           --input.cursorPosition_;
           break;
      case KEY_END:
           input.cursorPosition_ = input.value_.size();
           break;
      case KEY_HOME:
           input.cursorPosition_ = 0;
           break;
      case KEY_BACKSPACE:
           if( input.cursorPosition_ == 0 )
              break;
           input.value_.erase(input.cursorPosition_-1, 1);
           --input.cursorPosition_;
           break;
      case KEY_DC:  // delete key
           if( input.value_.empty() )
             break;
           input.value_.erase(input.cursorPosition_, 1);
           break;
    }
    if( input.value_.empty() )
      BUT_ASSERT( input.cursorPosition_ == 0);
    else
      BUT_ASSERT( input.cursorPosition_ <= input.value_.size() );
    // TODO: default adds character at a current position
    return Change::Update;
  }

  Change action(Field::Radio& radio, const unsigned row)
  {
    (void)row;
    switch( getch() )
    {
      case KEY_UP: return Change::Previous;
      case KEY_DOWN: return Change::Next;
      case 10:
      case KEY_ENTER: return Change::Next;
      case KEY_RIGHT: radio.selection_ = (radio.selection_ + 1) % radio.values_.size(); break;
      case KEY_LEFT:  radio.selection_ = (radio.selection_ == 0) ? radio.values_.size()-1u : radio.selection_-1; break;
      case 'q': return Change::Exit;
    }
    return Change::Update;
  }

  std::tuple<Fields...> fields_;
  But::NotNullUnique<Window> window_;   // TODO: direct value will suffice here?
};


template<typename ...Fields>
auto makeForm(Fields... fields)
{
  return Form<Fields...>{ std::move(fields)... };
}

}
