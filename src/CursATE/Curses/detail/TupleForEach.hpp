#pragma once
#include <tuple>

namespace CursATE::Curses::detail
{

template<unsigned I, unsigned N>
struct TupleForEach
{
  template<typename Tuple, typename F>
  static void visit(Tuple& t, F&& f)
  {
    f( std::get<I>(t) );
    TupleForEach<I+1, N>::visit(t, f);
  }
};

template<unsigned N>
struct TupleForEach<N,N>
{
  template<typename Tuple, typename F>
  static void visit(Tuple& t, F&& f)
  {
    (void)t;
    (void)f;
    throw std::logic_error{"requested index is out of bound"};
  }
};

}
