#pragma once
#include "CursATE/Curses/Change.hpp"
#include "CursATE/Curses/Window.hpp"
#include "CursATE/Curses/Field/Input.hpp"
#include "CursATE/Curses/Field/Radio.hpp"
#include "CursATE/Curses/Field/Button.hpp"
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

  std::string getResult(Field::Button const& button) const { return button.clicked_ ? "true" : "false"; }
  std::string getResult(Field::Input const& input) const { return input.value_; }
  std::string getResult(Field::Radio const& radio) const
  {
    BUT_ASSERT( radio.selection_ < radio.values_.size() && "inde out of bound" );
    return radio.values_[radio.selection_];
  }

  Change processElement(int n)
  {
    auto processor = [&](auto& e) { return this->action(e); };
    return detail::TupleVisitor<0, size()>::visit(n, fields_, processor);
  }

  Change action(Field::Button& button)
  {
    (void)button;
    // TODO
    return Change::Next;
  }
  Change action(Field::Input& input)
  {
    (void)input;
    // TODO
    return Change::Next;
  }
  Change action(Field::Radio& radio)
  {
    (void)radio;
    // TODO
    return Change::Next;
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
