#ifndef ET_EITHER_HPP_
#define ET_EITHER_HPP_

#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace et {

namespace detail {

struct PlaceSuccessType final {};
struct PlaceErrorType final {};

}  // namespace detail

constexpr detail::PlaceSuccessType PlaceSuccess{};
constexpr detail::PlaceErrorType PlaceError{};

class EitherException : public std::logic_error {
 public:
  EitherException(char const* msg) : std::logic_error(msg) {}
};

namespace detail {
namespace meta {

template <bool B>
using BoolConstant = std::integral_constant<bool, B>;

template <bool...>
struct BoolPack {};

template <bool... Bs>
using Conjuction = std::is_same<BoolPack<true, Bs...>, BoolPack<Bs..., true>>;

template <template <class> class F, class... Ts>
using All = Conjuction<F<Ts>::value...>;

}  // namespace meta

enum class Byte : std::uint8_t {};

// default case: `false`
template <class S, class E,
          bool = meta::All<std::is_trivially_destructible, S, E>::value>
class UnionStorage {
 public:
  using SuccessType = S;
  using ErrorType = E;

 protected:
  /*
   * unsafe, but union is never destructed in uninitialized context
   */
  ~UnionStorage() noexcept {
    if (is_success_) {
      succ_val_.~SuccessType();
    } else {
      err_val_.~ErrorType();
    }
  }

  constexpr UnionStorage() noexcept : null_state_() {}

  bool is_success_;
  union {
    Byte null_state_;
    SuccessType succ_val_;
    ErrorType err_val_;
  };
};

template <class S, class E>
class UnionStorage<S, E, true> {
 protected:
  using SuccessType = S;
  using ErrorType = E;

  constexpr UnionStorage() noexcept : null_state_() {}

  bool is_success_;
  union {
    Byte null_state_;
    SuccessType succ_val_;
    ErrorType err_val_;
  };
};

template <class, class>
class Storage;

template <class S>
class Storage<S, void> {
 protected:
  using SuccessType = S;
  using ErrorType = void;

  static constexpr auto kNoThrowSuccCopyCtor =
      std::is_nothrow_copy_constructible<SuccessType>::value;
  static constexpr auto kNoThrowSuccMoveCtor =
      std::is_nothrow_move_constructible<SuccessType>::value;

  template <class SS>
  static constexpr auto kNoThrowAssignableFrom =
      std::is_nothrow_assignable<SuccessType, SS>::value;

  template <class SS, class EE>
  static constexpr auto kCanNoThrowAssignThat =
      kNoThrowAssignableFrom<SS> && std::is_void<EE>::value;

 public:
  constexpr Storage(SuccessType const& succ_val) noexcept(kNoThrowSuccCopyCtor)
      : succ_val_(succ_val) {}

  constexpr Storage(SuccessType&& succ_val) noexcept(kNoThrowSuccMoveCtor)
      : succ_val_(std::move(succ_val)) {}

  template <class SS, class EE,
            class = std::enable_if_t<std::is_convertible<SS, S>::value>>
  constexpr Storage& operator=(Storage<SS, EE> const& that) noexcept(
      kCanNoThrowAssignThat<SS, EE>) {
    if (that.IsSuccess()) {
      succ_val_ = that.succ_val_;
    } else {
      throw EitherException("[et::Either] assigning non-success to succes");
    }

    return *this;
  }

  template <class SS, class EE,
            class = std::enable_if_t<std::is_convertible<SS, S>::value>>
  constexpr Storage& operator=(Storage<SS, EE>&& that) noexcept(
      kCanNoThrowAssignThat<SS, EE>) {
    if (that.IsSuccess()) {
      succ_val_ = std::move(that.succ_val_);
    } else {
      throw EitherException("[et::Either] assigning non-success to succes");
    }

    return *this;
  }

  constexpr bool IsSuccess() const noexcept { return true; }
  constexpr bool IsError() const noexcept { return false; }

 protected:
  SuccessType succ_val_;
};

template <class E>
class Storage<void, E> {
 protected:
  using SuccessType = void;
  using ErrorType = E;

  static constexpr auto kNoThrowSuccCopyCtor =
      std::is_nothrow_copy_constructible<ErrorType>::value;
  static constexpr auto kNoThrowSuccMoveCtor =
      std::is_nothrow_move_constructible<ErrorType>::value;

  template <class EE>
  static constexpr auto kNoThrowAssignableFrom =
      std::is_nothrow_assignable<ErrorType, EE>::value;

  template <class SS, class EE>
  static constexpr auto kCanNoThrowAssignThat =
      kNoThrowAssignableFrom<EE>and std::is_void<SS>::value;

 public:
  constexpr Storage(ErrorType const& err_val) noexcept(kNoThrowSuccCopyCtor)
      : err_val_(err_val) {}

  constexpr Storage(ErrorType&& err_val) noexcept(kNoThrowSuccMoveCtor)
      : err_val_(std::move(err_val)) {}

  template <class SS, class EE,
            class = std::enable_if_t<std::is_convertible<EE, E>::value>>
  constexpr Storage& operator=(Storage<SS, EE> const& that) noexcept(
      kCanNoThrowAssignThat<SS, EE>) {
    if (that.IsError()) {
      err_val_ = that.err_val_;
    } else {
      throw EitherException("[et::Either] assigning non-error to error");
    }
    return *this;
  }

  template <class SS, class EE,
            class = std::enable_if_t<std::is_convertible<EE, E>::value>>
  constexpr Storage& operator=(Storage<SS, EE>&& that) noexcept(
      kCanNoThrowAssignThat<SS, EE>) {
    if (that.IsError()) {
      err_val_ = std::move(that.err_val_);
    } else {
      throw EitherException("[et::Either] assigning non-error to error");
    }
    return *this;
  }

  constexpr bool IsSuccess() const noexcept { return false; }
  constexpr bool IsError() const noexcept { return true; }

 protected:
  ErrorType err_val_;
};

template <class S, class E>
class Storage : protected UnionStorage<S, E> {
 public:
  using SuccessType = S;
  using ErrorType = E;

  constexpr Storage() = delete;

 protected:
  template <class T>
  static constexpr auto kIsTriviallyDestructible =
      std::is_trivially_destructible<T>::value;

  template <class T>
  static constexpr auto kNoThrowCopyCtor =
      std::is_nothrow_copy_constructible<T>::value;

  template <class T>
  static constexpr auto kNoThrowMoveCtor =
      std::is_nothrow_move_constructible<T>::value;

 public:
  template <class = std::enable_if_t<kIsTriviallyDestructible<SuccessType>>>
  auto stuff() {}

  constexpr bool IsSuccess() const noexcept { return is_success_; }
  constexpr bool IsError() const noexcept { return !is_success_; }
};

}  // namespace detail

template <class S, class E>
class Either : private detail::Storage<S, E> {};

}  // namespace et

#endif  // ET_EITHER_HPP_
