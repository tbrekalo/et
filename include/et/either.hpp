#ifndef ET_EITHER_HPP_
#define ET_EITHER_HPP_

#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace et {

namespace detail {

struct SuccessTagImplType final {};
struct ErrorTagImplType final {};

}  // namespace detail

constexpr detail::SuccessTagImplType SuccessTag{};
constexpr detail::ErrorTagImplType ErrorTag{};

class EitherException : public std::logic_error {
 public:
  explicit EitherException(char const* msg) : std::logic_error(msg) {}
};

namespace detail {

using SuccessTagType = decltype(SuccessTag) const&;
using ErrorTagType = decltype(ErrorTag) const&;

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

template <class S, class E, bool>
class Storage;

template <class S>
class Storage<S, void, true> {
 public:
  using SuccessType = S;
  using ErrorType = void;

 protected:
  static constexpr auto kNoThrowSuccCopyCtor =
      std::is_nothrow_copy_constructible<SuccessType>::value;
  static constexpr auto kNoThrowSuccMoveCtor =
      std::is_nothrow_move_constructible<SuccessType>::value;

  Storage(SuccessTagType,
          SuccessType const& succ_val) noexcept(kNoThrowSuccCopyCtor)
      : succ_val_(succ_val) {}

  Storage(SuccessTagType, SuccessType&& succ_val) noexcept(kNoThrowSuccMoveCtor)
      : succ_val_(std::move(succ_val)) {}

 public:
  constexpr bool IsSuccess() const noexcept { return true; }
  constexpr bool IsError() const noexcept { return false; }

 protected:
  SuccessType succ_val_;
};

template <class E>
class Storage<void, E, true> {
 public:
  using SuccessType = void;
  using ErrorType = E;

 protected:
  static constexpr auto kNoThrowSuccCopyCtor =
      std::is_nothrow_copy_constructible<ErrorType>::value;
  static constexpr auto kNoThrowSuccMoveCtor =
      std::is_nothrow_move_constructible<ErrorType>::value;

  Storage(ErrorTagType, ErrorType const& err_val) noexcept(kNoThrowSuccCopyCtor)
      : err_val_(err_val) {}

  Storage(ErrorTagType, ErrorType&& err_val) noexcept(kNoThrowSuccMoveCtor)
      : err_val_(std::move(err_val)) {}

 public:
  constexpr bool IsSuccess() const noexcept { return false; }
  constexpr bool IsError() const noexcept { return true; }

 protected:
  ErrorType err_val_;
};

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
      decltype(SuccessTag) const&,
      SuccessType const& succ_val) noexcept(kNoThrowSuccCopyCtor)
      : is_success_(true), succ_val_(succ_val) {}

  explicit constexpr Storage(
      decltype(SuccessTag) const&,
      SuccessType&& succ_val) noexcept(kNoThrowSuccMoveCtor)
      : is_success_(true), succ_val_(std::move(succ_val)) {}

  explicit constexpr Storage(
      decltype(ErrorTag) const&,
      ErrorType const& err_val) noexcept(kNothrowErrCopyCtor)
      : is_success_(false), err_val_(err_val) {}

  explicit constexpr Storage(decltype(ErrorTag) const&,
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
      decltype(SuccessTag) const&,
      SuccessType const& succ_val) noexcept(kNoThrowSuccCopyCtor)
      : is_success_(true), succ_val_(succ_val) {}

  explicit constexpr Storage(
      decltype(SuccessTag) const&,
      SuccessType&& succ_val) noexcept(kNoThrowSuccMoveCtor)
      : is_success_(true), succ_val_(std::move(succ_val)) {}

  explicit constexpr Storage(
      decltype(ErrorTag) const&,
      ErrorType const& err_val) noexcept(kNothrowErrCopyCtor)
      : is_success_(false), err_val_(err_val) {}

  explicit constexpr Storage(decltype(ErrorTag) const&,
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
class Either final : public detail::Storage<S, E> {
 private:
  using Base = detail::Storage<S, E>;

  template <class... Args>
  constexpr Either(Args... args) noexcept(
      std::is_nothrow_constructible<Base, Args...>::value)
      : Base(std::forward<Args>(args)...) {}

 public:
  using SuccessType = typename Base::SuccessType;
  using ErrorType = typename Base::ErrorType;

  template <class SS>
  friend constexpr std::enable_if_t<
      detail::meta::Conjuction<
          !std::is_same<SuccessType, void>::value,
          std::is_same<SuccessType, std::decay_t<SS>>::value>::value,
      Either>
  Success(SS&& succ_val) noexcept(
      std::is_nothrow_constructible<Base, detail::SuccessTagType, SS>::value) {
    return Either(SuccessTag, std::forward<SS>(succ_val));
  }

  template <class EE>
  friend constexpr std::enable_if_t<
      detail::meta::Conjuction<
          !std::is_same<ErrorType, void>::value,
          std::is_same<ErrorType, std::decay_t<EE>>::value>::value,
      Either>
  Err(EE&& err_val) noexcept(
      std::is_nothrow_constructible<Base, detail::ErrorTagType, EE>::value){
      return Either(ErrorTag, std::forward<EE>(err_val));
  };
};

}  // namespace et

#endif  // ET_EITHER_HPP_
