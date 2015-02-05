#ifndef GENERIC_CIFY_HPP
# define GENERIC_CIFY_HPP
# pragma once

#include <utility>

namespace generic
{

namespace
{

template <typename F, int I, typename L, typename R, typename ...A>
inline F cify(L&& l, R (*)(A...))
{
  static L l_(::std::forward<L>(l));

  struct S
  {
    static R f(A... args) noexcept(noexcept(l_(::std::forward<A>(args)...)))
    {
      return l_(::std::forward<A>(args)...);
    }
  };

  static bool full;

  if (full)
  {
    l_.~L();
    new (static_cast<void*>(&l_)) L(::std::forward<L>(l));
  }
  // else do nothing

  full = true;

  return S::f;
}

}

template <typename F, int I = 0, typename L>
inline F cify(L&& l)
{
  return cify<F, I>(::std::forward<L>(l), F());
}

}

#endif // GENERIC_CIFY_HPP