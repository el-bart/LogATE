#pragma once
#include "CursATE/Curses/Change.hpp"
#include "CursATE/Curses/Exception.hpp"
#include "CursATE/Curses/FieldSize.hpp"
#include "CursATE/Curses/Window.hpp"
#include "CursATE/Curses/Field/Input.hpp"
#include "CursATE/Curses/Field/Radio.hpp"
#include "CursATE/Curses/Field/Button.hpp"
#include "CursATE/Curses/Field/detail/resizePadded.hpp"
#include "CursATE/Curses/detail/TupleVisitor.hpp"
#include "CursATE/Curses/detail/TupleForEach.hpp"
#include <But/Optional.hpp>
#include <tuple>
#include <array>
#include <map>

namespace CursATE::Curses
{

BUT_DEFINE_EXCEPTION(ScreenTooSmall, Exception, "screen too small");
BUT_DEFINE_EXCEPTION(ShortcutToUnknownField, Exception, "shortcut to unknown field");

using KeyShortcuts = std::map<char, std::string>; // key -> label/name

template<typename ...Fields>
struct Form final
{
  using Result = std::array<std::string, sizeof...(Fields)>;

  explicit Form(KeyShortcuts const& shortcuts, Fields&& ...fields):
    fields_{ std::forward<Fields>(fields)... },
    window_{ ScreenPosition{Row{0}, Column{0}}, ScreenSize::global(), Window::Boxed::True },
    shortcuts_{ convertToFieldsNumbersMap(shortcuts) }
  { }

  constexpr static auto size() { return sizeof...(Fields); }

  Result process()
  {
    selected_ = 0;
    while(true)
    {
      drawAll(selected_);
      const auto action = processElement(selected_);
      switch(action)
      {
        case Change::Update: break;
        case Change::Next: selected_ = (selected_+1) % size(); break;
        case Change::Previous: selected_ = (selected_ == 0) ? size()-1u : selected_-1; break;
        case Change::Exit: return prepareResult();
      }
    }
  }

private:
  using KeyShortcutsCompiled = std::map<char, unsigned>; // key -> filed number

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
    const auto uas = window_.userAreaSize();
    if( static_cast<unsigned>(uas.columns_.value_) < fs.label_ + 1u + fs.value_ )
      BUT_THROW(ScreenTooSmall, "available " << uas.columns_.value_ << " while required " << fs.label_ + 1 + fs.value_);
    fs.value_ = uas.columns_.value_ - 2u - fs.label_;
    return fs;
  }

  void drawAll(const int selected)
  {
    //window_.clear();
    const auto fs = calculateSpacing();
    auto uasp = window_.userAreaStartPosition();
    auto n = 0;
    auto draw = [&](auto const& e)
        {
          const auto nowSelected = selected==n;
          Field::draw(window_, uasp, fs, e, nowSelected);
          ++uasp.row_.value_;
          ++n;
        };
    detail::TupleForEach<0, size()>::visit(fields_, draw);
    window_.refresh();
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
    const auto ch = getch();
    switch(ch)
    {
      case KEY_UP: return Change::Previous;
      case KEY_DOWN: return Change::Next;
      case ' ':
      case 10:
      case KEY_ENTER: button.clicked_ = true; return Change::Exit;
    }
    return tryProcessingAsShortcut(ch);
  }

  void positionCursorInInputField(Field::Input const& input, const unsigned row)
  {
    const auto fs = calculateSpacing();
    const auto uasp = window_.userAreaStartPosition();
    const auto vs = Field::detail::resizePaddedVisibleSize(input.value_, fs.value_, input.cursorPosition_);
    const auto valueStartPos = uasp.column_.value_ + fs.label_ + 1u + vs.selectionOffset_;
    wmove(window_.get(), uasp.row_.value_ + row, valueStartPos);
    window_.refresh();
  }

  Change action(Field::Input& input, const unsigned row)
  {
    positionCursorInInputField(input, row);
    const CursorVisibilityGuard cvg{CursorVisibility::Normal};
    const auto ch = getch();
    switch(ch)
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
    // sanity checks
    if( input.value_.empty() )
      BUT_ASSERT( input.cursorPosition_ == 0);
    else
      BUT_ASSERT( input.cursorPosition_ <= input.value_.size() );
    // by default add character to the string
    if( isprint(ch) )
    {
      input.value_.insert(input.cursorPosition_, 1, ch);
      ++input.cursorPosition_;
    }
    return Change::Update;
  }

  Change action(Field::Radio& radio, const unsigned row)
  {
    (void)row;
    const auto ch = getch();
    switch(ch)
    {
      case KEY_UP: return Change::Previous;
      case KEY_DOWN: return Change::Next;
      case 10:
      case KEY_ENTER: return Change::Next;
      case KEY_RIGHT: radio.selection_ = (radio.selection_ + 1) % radio.values_.size(); break;
      case KEY_LEFT:  radio.selection_ = (radio.selection_ == 0) ? radio.values_.size()-1u : radio.selection_-1; break;
    }
    return tryProcessingAsShortcut(ch);
  }

  KeyShortcutsCompiled convertToFieldsNumbersMap(KeyShortcuts const& in) const
  {
    KeyShortcutsCompiled out;
    for(auto& e: in)
    {

      auto pos = 0u;
      auto isSet = false;
      auto str2pos = [&](auto const& f)
        {
          if( f.label_ == e.second )
          {
            out[e.first] = pos;
            isSet = true;
          }
          ++pos;
        };
      detail::TupleForEach<0, size()>::visit(fields_, str2pos);
      if(not isSet)
        BUT_THROW(ShortcutToUnknownField, "key '" << e.first << "' maps to unkonw field '" << e.second << "'");
    }
    return out;
  }

  Change takeShortcut(int n)
  {
    auto processor = [&](auto& e) { return this->shortcutAction(e, n); };
    return detail::TupleVisitor<0, size()>::visit(n, fields_, processor);
  }

  Change shortcutAction(Field::Button& button, unsigned n)
  {
    (void)n;
    button.clicked_ = true;
    return Change::Exit;
  }

  Change shortcutAction(Field::Input& input, unsigned n)
  {
    (void)input;
    selected_ = n;
    return Change::Update;
  }

  Change shortcutAction(Field::Radio& radio, unsigned n)
  {
    (void)radio;
    selected_ = n;
    return Change::Update;
  }

  Change tryProcessingAsShortcut(int ch)
  {
    const auto it = shortcuts_.find(ch);
    if( it == end(shortcuts_) )
      return Change::Update;
    return takeShortcut(it->second);
  }

  std::tuple<Fields...> fields_;
  Window window_;
  const KeyShortcutsCompiled shortcuts_;
  unsigned selected_{0};
};


template<typename ...Fields>
auto makeForm(KeyShortcuts const& shortcuts, Fields... fields)
{
  return Form<Fields...>{ shortcuts, std::move(fields)... };
}

}
