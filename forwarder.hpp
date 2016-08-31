#ifndef GENERIC_FORWARDER_HPP
# define GENERIC_FORWARDER_HPP
# pragma once

#include <cassert>

// ::std::size_t
#include <cstddef>

#include <functional>

#include <type_traits>

#include <utility>

namespace generic
{

namespace detail
{

namespace forwarder
{

template <typename ...A>
struct argument_types
{
};

template <typename A>
struct argument_types<A>
{
  using argument_type = A;
};

template <typename A, typename B>
struct argument_types<A, B>
{
  using first_argument_type = A;
  using second_argument_type = B;
};

}

}

constexpr auto const default_forwarder_noexcept =
#if __cpp_exceptions
false;
#else
true;
#endif // __cpp_exceptions

constexpr auto const default_forwarder_size = 4 * sizeof(void*);

template <typename F,
  ::std::size_t N = default_forwarder_size,
  bool NE = default_forwarder_noexcept
>
class forwarder;

template <typename R, typename ...A, ::std::size_t N, bool NE>
class forwarder<R (A...), N, NE> : public detail::forwarder::argument_types<A...>
{
  R (*stub_)(void const*, A&&...) noexcept(NE) {};

  ::std::aligned_storage_t<N> store_;

  template<typename T, typename ...U, ::std::size_t M>
  friend bool operator==(forwarder<T (U...), M> const&,
    ::std::nullptr_t) noexcept;
  template<typename T, typename ...U, ::std::size_t M>
  friend bool operator==(::std::nullptr_t,
    forwarder<T (U...), M> const&) noexcept;

  template<typename T, typename ...U, ::std::size_t M>
  friend bool operator!=(forwarder<T (U...), M> const&,
    ::std::nullptr_t) noexcept;
  template<typename T, typename ...U, ::std::size_t M>
  friend bool operator!=(::std::nullptr_t,
    forwarder<T (U...), M> const&) noexcept;

public:
  using result_type = R;

public:
  forwarder() = default;

  forwarder(forwarder const&) = default;

  forwarder(forwarder&&) = default;

  template <typename T>
  forwarder(T&& t) noexcept
  {
    assign(::std::forward<T>(t));
  }

  forwarder& operator=(forwarder const&) = default;

  forwarder& operator=(forwarder&&) = default;

  template <typename T>
  auto& operator=(T&& f) noexcept
  {
    assign(::std::forward<T>(f));

    return *this;
  }

  explicit operator bool() const noexcept { return stub_; }

  R operator()(A... args) const
    noexcept(noexcept(stub_(&store_, ::std::forward<A>(args)...)))
  {
    //assert(stub_);
    return stub_(&store_, ::std::forward<A>(args)...);
  }

  void assign(::std::nullptr_t) noexcept
  {
    reset();
  }

  template <typename T>
  void assign(T&& f) noexcept
  {
    using functor_type = ::std::decay_t<T>;

    static_assert(sizeof(functor_type) <= sizeof(store_),
      "functor too large");
    static_assert(::std::is_trivially_copyable<functor_type>{},
      "functor not trivially copyable");

    ::new (static_cast<void*>(&store_)) functor_type(::std::forward<T>(f));

    stub_ = [](void const* const ptr, A&&... args) noexcept(
          noexcept(
          (
#if __cplusplus <= 201402L
            (*static_cast<functor_type const*>(ptr))(
              ::std::forward<A>(args)...)
#else
            ::std::invoke(*static_cast<functor_type const*>(ptr),
              ::std::forward<A>(args)...)
#endif // __cplusplus
          )
        )
      ) -> R
      {
#if __cplusplus <= 201402L
        return (*static_cast<functor_type const*>(ptr))(
          ::std::forward<A>(args)...);
#else
        return ::std::invoke(*static_cast<functor_type const*>(ptr),
          ::std::forward<A>(args)...);
#endif // __cplusplus
      };
  }

  void reset() noexcept { stub_ = nullptr; }

  void swap(forwarder& other) noexcept { ::std::swap(*this, other); }

  template <typename T>
  auto target() noexcept
  {
    return reinterpret_cast<T*>(&store_);
  }

  template <typename T> 
  auto target() const noexcept
  {
    return reinterpret_cast<T const*>(&store_);
  }
};

template<typename R, typename ...A, ::std::size_t N>
bool operator==(forwarder<R (A...), N> const& f,
  ::std::nullptr_t const) noexcept
{
  return f.stub_ == nullptr;
}

template<typename R, typename ...A, ::std::size_t N>
bool operator==(::std::nullptr_t const,
  forwarder<R (A...), N> const& f) noexcept
{
  return f.stub_ == nullptr;
}

template<typename R, typename ...A, ::std::size_t N>
bool operator!=(forwarder<R (A...), N> const& f,
  ::std::nullptr_t const) noexcept
{
  return !operator==(f, nullptr);
}

template<typename R, typename ...A, ::std::size_t N>
bool operator!=(::std::nullptr_t const,
  forwarder<R (A...), N> const& f) noexcept
{
  return !operator==(f, nullptr);
}

}

#endif // GENERIC_FORWARDER_HPP
