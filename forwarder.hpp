#ifndef GENERIC_FORWARDER_HPP
# define GENERIC_FORWARDER_HPP
# pragma once

#include <cassert>

// ::std::size_t
#include <cstddef>

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

template<typename F, ::std::size_t N = 4 * sizeof(void*)>
class forwarder;

template<typename R, typename ...A, ::std::size_t N>
class forwarder<R (A...), N> : public detail::forwarder::argument_types<A...>
{
  R (*stub_)(void const*, A&&...){};

  typename ::std::aligned_storage<N>::type store_;

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
  using result_type	= R;

public:
  forwarder() = default;

  forwarder(forwarder const&) = default;

  template<typename T, typename ...U,
    typename = typename ::std::enable_if<
      !::std::is_same<forwarder, typename ::std::decay<T>::type>{}
    >::type
  >
  forwarder(T&& t, U&& ...u) noexcept
  {
    assign(::std::forward<T>(t), ::std::forward<U>(u)...);
  }

  forwarder& operator=(forwarder const&) = default;

  forwarder& operator=(forwarder&&) = default;

  template<typename T, typename ...U,
    typename = typename ::std::enable_if<
      !::std::is_same<forwarder, typename ::std::decay<T>::type>{}
    >::type
  >
  forwarder(T&& t, U&& ...u) noexcept
  {
    assign(::std::forward<T>(t), ::std::forward<U>(u)...);

    return *this;
  }

  explicit operator bool() const noexcept { return stub_; }

  R operator()(A... args) const
    noexcept(noexcept(stub_(&store_, ::std::forward<A>(args)...)))
  {
    //assert(stub_);
    return stub_(&store_, ::std::forward<A>(args)...);
  }

  template <typename T>
  void assign(T&& f) noexcept
  {
    using functor_type = typename ::std::decay<T>::type;

    static_assert(sizeof(functor_type) <= sizeof(store_),
      "functor too large");
    static_assert(::std::is_trivially_copyable<T>{},
      "functor not trivially copyable");

    ::new (static_cast<void*>(&store_)) functor_type(::std::forward<T>(f));

    stub_ = [](void const* const ptr, A&&... args) noexcept(noexcept(
        (*static_cast<functor_type const*>(ptr))(::std::forward<A>(args)...))
      ) -> R
      {
        return (*static_cast<functor_type const*>(ptr))(
          ::std::forward<A>(args)...
        );
      };
  }

  template <class C>
  void assign(C* const object_ptr,
    R (C::* const method_ptr)(A...)) noexcept
  {
    assign([object_ptr, method_ptr](A&&... args) noexcept(
      (object_ptr->*method_ptr)(::std::declval<A>()...)) {
        return (object_ptr->*method_ptr)(::std::forward<A>(args)...);
      }
    );
  }

  template <class C>
  void assign(C* const object_ptr,
    R (C::* const method_ptr)(A...) const) noexcept
  {
    assign([object_ptr, method_ptr](A&&... args) noexcept(
      (object_ptr->*method_ptr)(::std::declval<A>()...)) {
        return (object_ptr->*method_ptr)(::std::forward<A>(args)...);
      }
    );
  }

  template <class C>
  void assign(C& object,
    R (C::* const method_ptr)(A...)) noexcept
  {
    assign([&object, method_ptr](A&&... args) noexcept(
      (object.*method_ptr)(::std::declval<A>()...)) {
        return (object.*method_ptr)(::std::forward<A>(args)...);
      }
    );
  }

  template <class C>
  void assign(C const& object,
    R (C::* const method_ptr)(A...) const) noexcept
  {
    assign([&object, method_ptr](A&&... args) noexcept(
      (object.*method_ptr)(::std::declval<A>()...)) {
        return (object.*method_ptr)(::std::forward<A>(args)...);
      }
    );
  }

  void reset() noexcept { stub_ = nullptr; }

  void swap(forwarder& other) noexcept { ::std::swap(*this, other); }

  template <typename T>
  T* target() noexcept
  {
    return reinterpret_cast<T*>(&store_);
  }

  template <typename T> 
  T const* target() const noexcept
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
