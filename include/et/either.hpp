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
  explicit EitherException(char const* msg) : std::logic_error(msg) {}
};

namespace detail {

using SuccessTagType = decltype(PlaceSuccess) const&;
using ErrorTagType = decltype(PlaceError) const&;

namespace meta {

template <bool B>
using BoolConstant = std::integral_constant<bool, B>;

template <bool...>
struct BoolPack {};

template <bool... Bs>
using Conjuction = std::is_same<BoolPack<true, Bs...>, BoolPack<Bs..., true>>;

template <bool... Bs>
using Disjunction = BoolConstant<!Conjuction<!Bs...>::value>;

template <template <class> class F, class... Ts>
using All = Conjuction<F<Ts>::value...>;

}  // namespace meta

// default case: false
template <class S, class E,
          bool = meta::All<std::is_trivially_destructible, S, E>::value>
class Storage {
 public:
  using SuccessType = S;
  using ErrorType = E;

  constexpr Storage() = delete;

  ~Storage() noexcept(
      meta::All<std::is_nothrow_destructible, SuccessType, ErrorType>::value) {
    if (is_success_) {
      succ_val_.~SuccessType();
    } else {
      err_val_.~ErrorType();
    }
  }

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

  static constexpr auto kNoThrowSuccCopyCtor = kNoThrowCopyCtor<SuccessType>;
  static constexpr auto kNoThrowSuccMoveCtor = kNoThrowMoveCtor<SuccessType>;

  static constexpr auto kNothrowErrCopyCtor = kNoThrowCopyCtor<ErrorType>;
  static constexpr auto kNoThrowErrMoveCtor = kNoThrowMoveCtor<ErrorType>;

  explicit constexpr Storage(
      decltype(PlaceSuccess) const&,
      SuccessType const& succ_val) noexcept(kNoThrowSuccCopyCtor)
      : is_success_(true), succ_val_(succ_val) {}

  explicit constexpr Storage(
      decltype(PlaceSuccess) const&,
      SuccessType&& succ_val) noexcept(kNoThrowSuccMoveCtor)
      : is_success_(true), succ_val_(std::move(succ_val)) {}

  explicit constexpr Storage(
      decltype(PlaceError) const&,
      ErrorType const& err_val) noexcept(kNothrowErrCopyCtor)
      : is_success_(false), err_val_(err_val) {}

  explicit constexpr Storage(decltype(PlaceError) const&,
                             ErrorType&& err_val) noexcept(kNoThrowErrMoveCtor)
      : is_success_(false), err_val_(std::move(err_val)) {}

 public:
  constexpr bool IsSuccess() const noexcept { return is_success_; }
  constexpr bool IsError() const noexcept { return !is_success_; }

 protected:
  bool is_success_;
  union {
    SuccessType succ_val_;
    ErrorType err_val_;
  };
};

template <class S, class E>
class Storage<S, E, true> {
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

  static constexpr auto kNoThrowSuccCopyCtor = kNoThrowCopyCtor<SuccessType>;
  static constexpr auto kNoThrowSuccMoveCtor = kNoThrowMoveCtor<SuccessType>;

  static constexpr auto kNothrowErrCopyCtor = kNoThrowCopyCtor<ErrorType>;
  static constexpr auto kNoThrowErrMoveCtor = kNoThrowMoveCtor<ErrorType>;

  explicit constexpr Storage(
      decltype(PlaceSuccess) const&,
      SuccessType const& succ_val) noexcept(kNoThrowSuccCopyCtor)
      : is_success_(true), succ_val_(succ_val) {}

  explicit constexpr Storage(
      decltype(PlaceSuccess) const&,
      SuccessType&& succ_val) noexcept(kNoThrowSuccMoveCtor)
      : is_success_(true), succ_val_(std::move(succ_val)) {}

  explicit constexpr Storage(
      decltype(PlaceError) const&,
      ErrorType const& err_val) noexcept(kNothrowErrCopyCtor)
      : is_success_(false), err_val_(err_val) {}

  explicit constexpr Storage(decltype(PlaceError) const&,
                             ErrorType&& err_val) noexcept(kNoThrowErrMoveCtor)
      : is_success_(false), err_val_(std::move(err_val)) {}

 public:
  constexpr bool IsSuccess() const noexcept { return is_success_; }
  constexpr bool IsError() const noexcept { return !is_success_; }

 protected:
  bool is_success_;
  union {
    SuccessType succ_val_;
    ErrorType err_val_;
  };
};

}  // namespace detail

template <class S, class E>
class Either;

template <class S>
class Either<S, void> {
 public:
  using SuccessType = S;
  using ErrorType = void;

 private:
  static constexpr auto kNoThrowSuccCopyCtor =
      std::is_nothrow_copy_constructible<SuccessType>::value;
  static constexpr auto kNoThrowSuccMoveCtor =
      std::is_nothrow_move_constructible<SuccessType>::value;

  template <class SS>
  static constexpr auto kNoThrowAssignableFrom =
      std::is_nothrow_assignable<SuccessType, SS>::value;

  template <class SS, class EE>
  static constexpr auto kCanNoThrowAssignThat =
      kNoThrowAssignableFrom<SS>&& std::is_void<EE>::value;

  explicit constexpr Either(SuccessType const& succ_val) noexcept(
      kNoThrowSuccCopyCtor)
      : succ_val_(succ_val) {}

  explicit constexpr Either(SuccessType&& succ_val) noexcept(
      kNoThrowSuccMoveCtor)
      : succ_val_(std::move(succ_val)) {}

 public:
  template <class EE>
  constexpr Either& operator=(Either<SuccessType, EE> const& that) noexcept(
      kCanNoThrowAssignThat<SuccessType, EE>) {
    if (that.IsSuccess()) {
      succ_val_ = that.succ_val_;
    } else {
      throw EitherException("[et::Either] assigning non-success to success");
    }

    return *this;
  }

  template <class EE>
  constexpr Either& operator=(Either<SuccessType, EE>&& that) noexcept(
      kCanNoThrowAssignThat<SuccessType, EE>) {
    if (that.IsSuccess()) {
      succ_val_ = std::move(that.succ_val_);
    } else {
      throw EitherException("[et::Either] assigning non-success to success");
    }

    return *this;
  }

  template <class SS>
  friend constexpr std::enable_if_t<
      std::is_same<SuccessType, std::decay_t<SS>>::value, Either>
  Success(SS&& succ_val) noexcept(
      std::is_nothrow_constructible<Either, SS>::value) {
    return Either(std::forward<SS>(succ_val));
  }

  constexpr bool IsSuccess() const noexcept { return true; }
  constexpr bool IsError() const noexcept { return false; }

 protected:
  SuccessType succ_val_;
};

template <class E>
class Either<void, E> {
 public:
  using SuccessType = void;
  using ErrorType = E;

 private:
  static constexpr auto kNoThrowErrCopyCtor =
      std::is_nothrow_copy_constructible<ErrorType>::value;
  static constexpr auto kNoThrowErrMoveCtor =
      std::is_nothrow_move_constructible<ErrorType>::value;

  template <class EE>
  static constexpr auto kNoThrowAssignableFrom =
      std::is_nothrow_assignable<ErrorType, EE>::value;

  template <class SS, class EE>
  static constexpr auto kCanNoThrowAssignThat =
      kNoThrowAssignableFrom<EE>and std::is_void<SS>::value;

 public:
  explicit constexpr Either(ErrorType const& err_val) noexcept(
      kNoThrowErrCopyCtor)
      : err_val_(err_val) {}

  explicit constexpr Either(ErrorType&& err_val) noexcept(kNoThrowErrMoveCtor)
      : err_val_(std::move(err_val)) {}

  template <class SS>
  constexpr Either& operator=(Either<SS, ErrorType> const& that) noexcept(
      kCanNoThrowAssignThat<SS, ErrorType>) {
    if (that.IsError()) {
      err_val_ = that.err_val_;
    } else {
      throw EitherException("[et::Either] assigning non-error to error");
    }
    return *this;
  }

  template <class SS>
  constexpr Either& operator=(Either<SS, ErrorType>&& that) noexcept(
      kCanNoThrowAssignThat<SS, ErrorType>) {
    if (that.IsError()) {
      err_val_ = std::move(that.err_val_);
    } else {
      throw EitherException("[et::Either] assigning non-error to error");
    }
    return *this;
  }

  template <class EE>
  friend constexpr std::enable_if_t<
      std::is_same<ErrorType, std::decay_t<EE>>::value, Either>
  Success(EE&& succ_val) noexcept(
      std::is_nothrow_constructible<Either, EE>::value) {
    return Either(std::forward<EE>(succ_val));
  }

  constexpr bool IsSuccess() const noexcept { return false; }
  constexpr bool IsError() const noexcept { return true; }

 private:
  ErrorType err_val_;
};

template <class S, class E>
class Either {

};

}  // namespace et

#endif  // ET_EITHER_HPP_
