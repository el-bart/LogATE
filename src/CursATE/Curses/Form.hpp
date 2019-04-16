#pragma once
#include "CursATE/Curses/Window.hpp"
#include "CursATE/Curses/Field/Input.hpp"
#include "CursATE/Curses/Field/Radio.hpp"
#include "CursATE/Curses/Field/Button.hpp"
#include "CursATE/Curses/detail/TupleVisitor.hpp"
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
    window_{ But::makeUniqueNN<Window>(ScreenPosition{Row{1}, Column{1}}, ScreenSize::global(), Window::Boxed::True) }
  { }

  constexpr static auto size() { return sizeof...(Fields); }

  Result process()
  {
    auto selected = 0;
    while(true)
    {
      const auto action = processElement(selected);
      switch(action)
      {
        case Select::Next: selected = (selected+1) % size(); break;
        case Select::Previous: selected = (selected == 0) ? size()-1u : selected-1; break;
        case Select::Exit: return prepareResult();
      }
    }
  }

private:
  enum class Select
  {
    Next,
    Previous,
    Exit
  };

  Result prepareResult()
  {
    Result out;
    auto writer = [&](auto const& e) { return this->getResult(e); };
    for(auto i=0u; i<size(); ++i)
      detail::TupleVisitor<0, size()>::visit(i, fields_, writer);
    return out;
  }

  std::string getResult(Field::Button const& button) const { return button.clicked_ ? "true" : "false"; }
  std::string getResult(Field::Input const& input) const { return input.value_; }
  std::string getResult(Field::Radio const& radio) const
  {
    BUT_ASSERT( radio.selection_ < radio.values_.size() && "inde out of bound" );
    return radio.values_[radio.selection_];
  }

  Select processElement(int n)
  {
    auto processor = [&](auto& e) { return this->action(e); };
    return detail::TupleVisitor<0, size()>::visit(n, fields_, processor);
  }

  Select action(Field::Button& button)
  {
    (void)button;
    // TODO
    return Select::Next;
  }
  Select action(Field::Input& input)
  {
    (void)input;
    // TODO
    return Select::Next;
  }
  Select action(Field::Radio& radio)
  {
    (void)radio;
    // TODO
    return Select::Next;
  }

  std::tuple<Fields...> fields_;
  But::NotNullUnique<Window> window_;
};


template<typename ...Fields>
auto makeForm(Fields... fields)
{
  return Form<Fields...>{ std::move(fields)... };
}

}
