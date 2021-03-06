#ifndef GNR_IS_INVOKABLE_HPP
# define GNR_IS_INVOKABLE_HPP
# pragma once

namespace gnr
{

namespace
{

template <typename F, typename ...A>
auto f(int) -> decltype((void)::std::declval<F>()(::std::declval<A>()...),
  ::std::true_type{}
);

template <typename F, typename ...A>
auto f(long) -> ::std::false_type;

}

template <typename F, typename ...A>
struct is_invokable : decltype(f<F, A...>(0))
{
};

}

#endif // GNR_IS_INVOKABLE_HPP
