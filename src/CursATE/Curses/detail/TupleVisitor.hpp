#pragma once
#include <tuple>

namespace CursATE::Curses::detail
{

template<unsigned I, unsigned N>
struct TupleVisitor
{
  template<typename Tuple, typename F>
  static auto visit(int n, Tuple& t, F&& f)
  {
    if(I == n)
      return f( std::get<I>(t) );
    return TupleVisitor<I+1, N>::visit(n, t, f);
  }
};

template<unsigned N>
struct TupleVisitor<N,N>
{
  template<typename Tuple, typename F>
  static auto visit(int n, Tuple& t, F&& f) -> decltype(f(std::get<0>(t)))
  {
    (void)n;
    (void)t;
    (void)f;
    throw std::logic_error{"requested index is out of bound"};
  }
};

}
