#ifndef ET_EITHER_HPP_
#define ET_EITHER_HPP_

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace et {

template <class S, class E>
class Either;

namespace detail {

struct SuccessTagImpl {};
struct ErrorTagImpl {};

using SuccessTagType = SuccessTagImpl const&;
using ErrorTagType = ErrorTagImpl const&;

constexpr detail::SuccessTagImpl SuccessTag;
constexpr detail::ErrorTagImpl ErrorTag;

}  // namespace detail

class BadEitherAccess : public std::logic_error {
 public:
  explicit BadEitherAccess(char const* msg) : std::logic_error(msg) {}
};

class BadEitherAssign : public std::logic_error {
 public:
  explicit BadEitherAssign(char const* msg) : std::logic_error(msg) {}
};

namespace detail {
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

template <template <class> class F, class... Ts>
using Non = BoolConstant<!Disjunction<F<Ts>::value...>::value>;

template <class T>
using NotVoid = BoolConstant<!std::is_void<T>::value>;

template <class...>
using VoidType = void;

template <class T, class = VoidType<>>
struct IsPrintable : std::false_type {};

template <class T>
struct IsPrintable<T, VoidType<decltype(std::cout << ::std::declval<T>())>>
    : std::true_type {};

template <class T, class = VoidType<>>
struct IsStdHashable : std::false_type {};

template <class T>
struct IsStdHashable<
    T, VoidType<decltype(std::declval<std::hash<T>()(std::declval<T>())>)>>
    : std::true_type {};

}  // namespace meta

enum class StorageState { kEmpty, kHasError, kHasSuccess };

// default case for trivially destructible
template <class S, class E,
          bool = meta::All<std::is_trivially_destructible, S, E>::value>
class Storage {
 public:
  using SuccessType = S;
  using ErrorType = E;

 protected:
  template <class SS = SuccessType, class EE = ErrorType,
            class = std::enable_if_t<
                meta::All<std::is_copy_assignable, SS, EE>::value>>
  constexpr Storage operator=(Storage const& that) noexcept(
      meta::All<std::is_nothrow_copy_assignable, SS, EE>::value) {
    state_ = that.state_;
    if (state_ == StorageState::kHasSuccess) {
      succ_val_ = that.succ_val_;
    } else if (state_ == StorageState::kHasError) {
      err_val_ = that.err_val_;
    }
  }

  template <class SS = SuccessType, class EE = ErrorType,
            class = std::enable_if_t<
                meta::All<std::is_move_assignable, SS, EE>::value>>
  constexpr Storage(Storage&& that) noexcept(
      meta::All<std::is_nothrow_move_assignable, SS, EE>::value)
      : state_(that.state_) {
    state_ = that.state_;
    if (state_ == StorageState::kHasSuccess) {
      succ_val_ = std::move(that.succ_val_);
      that.state_ = StorageState::kEmpty;
    } else if (state_ == StorageState::kHasError) {
      err_val_ = std::move(that.err_val_);
      that.state_ = StorageState::kEmpty;
    }
  }

  template <class SS = SuccessType,
            class = std::enable_if_t<std::is_copy_constructible<SS>::value>>
  constexpr Storage(SuccessTagType, SuccessType const& succ_val) noexcept(
      std::is_nothrow_copy_constructible<SuccessType>::value)
      : state_(StorageState::kHasSuccess), succ_val_(succ_val) {}

  template <class SS = SuccessType,
            class = std::enable_if_t<std::is_move_constructible_v<SS>>>
  constexpr Storage(SuccessTagType, SuccessType&& succ_val) noexcept(
      std::is_nothrow_move_constructible_v<SuccessType>)
      : state_(StorageState::kHasSuccess), succ_val_(std::move(succ_val)) {}

  template <class EE = ErrorType,
            class = std::enable_if_t<std::is_copy_constructible_v<EE>>>
  constexpr Storage(ErrorTagType, ErrorType const& err_val) noexcept(
      std::is_nothrow_copy_constructible<ErrorType>::value)
      : state_(StorageState::kHasError), err_val_(err_val) {}

  template <class EE = ErrorType,
            class = std::enable_if_t<std::is_move_constructible_v<EE>>>
  constexpr Storage(ErrorTagType, ErrorType&& err_val) noexcept(
      std::is_nothrow_move_constructible_v<ErrorType>)
      : state_(StorageState::kHasError), err_val_(std::move(err_val)) {}

  StorageState state_;
  union {
    SuccessType succ_val_;
    ErrorType err_val_;
  };
};

template <class S, class E>
class Storage<S, E, false> {
 public:
  using SuccessType = S;
  using ErrorType = E;

 protected:
  template <class SS = SuccessType, class EE = ErrorType,
            class = std::enable_if_t<
                meta::All<std::is_copy_assignable, SS, EE>::value>>
  constexpr Storage operator=(Storage const& that) noexcept(
      meta::All<std::is_nothrow_copy_assignable, SS, EE>::value) {
    state_ = that.state_;
    if (state_ == StorageState::kHasSuccess) {
      succ_val_ = that.succ_val_;
    } else if (state_ == StorageState::kHasError) {
      err_val_ = that.err_val_;
    }
  }

  template <class SS = SuccessType, class EE = ErrorType,
            class = std::enable_if_t<
                meta::All<std::is_move_assignable, SS, EE>::value>>
  constexpr Storage(Storage&& that) noexcept(
      meta::All<std::is_nothrow_move_assignable, SS, EE>::value) {
    state_ = that.state_;
    if (state_ == StorageState::kHasSuccess) {
      succ_val_ = std::move(that.succ_val_);
      that.state_ = StorageState::kEmpty;
    } else if (state_ == StorageState::kHasError) {
      err_val_ = std::move(that.err_val_);
      that.state_ = StorageState::kEmpty;
    }
  }

  template <class SS = SuccessType,
            class = std::enable_if_t<std::is_copy_constructible_v<SS>>>
  constexpr Storage(SuccessTagType, SuccessType const& succ_val) noexcept(
      std::is_nothrow_copy_constructible_v<SuccessType>)
      : state_(StorageState::kHasSuccess), succ_val_(succ_val) {}

  template <class SS = SuccessType,
            class = std::enable_if_t<std::is_move_constructible_v<SS>>>
  constexpr Storage(SuccessTagType, SuccessType&& succ_val) noexcept(
      std::is_nothrow_move_constructible_v<SuccessType>)
      : state_(StorageState::kHasSuccess), succ_val_(std::move(succ_val)) {}

  template <class EE = ErrorType,
            class = std::enable_if_t<std::is_copy_constructible_v<EE>>>
  constexpr Storage(ErrorTagType, ErrorType const& err_val) noexcept(
      std::is_nothrow_copy_constructible<ErrorType>::value)
      : state_(StorageState::kHasError), err_val_(err_val) {}

  template <class EE = ErrorType,
            class = std::enable_if_t<std::is_move_constructible_v<EE>>>
  constexpr Storage(ErrorTagType, ErrorType&& err_val) noexcept(
      std::is_nothrow_move_constructible_v<ErrorType>)
      : state_(StorageState::kHasError), err_val_(std::move(err_val)) {}

  ~Storage() noexcept(
      meta::All<std::is_nothrow_destructible, SuccessType, ErrorType>::value) {
    if (state_ == StorageState::kHasSuccess) {
      succ_val_.~SuccessType();
    } else if (state_ == StorageState::kHasError) {
      err_val_.~ErrorType();
    }
  }

  StorageState state_;
  union {
    SuccessType succ_val_;
    ErrorType err_val_;
  };
};

template <class S, class E>
struct EitherConstraints {
  using SuccessType = S;
  using ErrorType = E;

  static_assert(
      not detail::meta::Conjuction<std::is_void<SuccessType>::value,
                                   std::is_void<ErrorType>::value>::value,
      "[et::Either] Either<void, void> ill formed");

  static_assert(not std::is_reference_v<SuccessType>,
                "[et::Either] Either<SuccessType&, ErrorType> ill formed");

  static_assert(not std::is_reference_v<ErrorType>,
                "[et::Either] Either<SuccessType, ErrorType> ill formed");

  static_assert(
      std::conditional_t<meta::NotVoid<S>::value, std::is_object<S>,
                         std::true_type>::value,
      "[et::Either] Either<SuccessType, ErrorType> only object type supported");
};

}  // namespace detail

template <class S>
class Either<S, void> final : detail::EitherConstraints<S, void> {
 public:
  using SuccessType = S;
  using ErrorType = void;

  Either() = delete;

  template <class SS = SuccessType,
            class = std::enable_if_t<std::is_copy_constructible<SS>::value>>
  explicit constexpr Either(SuccessType const& succ_val) noexcept(
      std::is_nothrow_copy_constructible<SuccessType>::value)
      : succ_val_(succ_val) {}

  template <class SS = SuccessType,
            class = std::enable_if_t<std::is_move_constructible<SS>::value>>
  explicit constexpr Either(SuccessType&& succ_val) noexcept(
      std::is_nothrow_move_constructible<ErrorType>::value)
      : succ_val_(std::move(succ_val)) {}

  constexpr auto IsSuccess() const noexcept -> bool { return true; }
  constexpr auto IsError() const noexcept -> bool { return false; }

  constexpr operator bool() const noexcept { return true; }

  //  constexpr auto Success() & noexcept -> SuccessType& { return succ_val_; }
  constexpr auto Success() const& noexcept -> SuccessType const& {
    return succ_val_;
  }

  constexpr auto Success() && noexcept -> SuccessType&& {
    return std::move(succ_val_);
  }
  constexpr auto Success() const&& noexcept -> SuccessType const&& {
    return std::move(succ_val_);
  }

  constexpr auto Error() const -> ErrorType {
    throw BadEitherAccess("[et::Either<S, void>::Error]");
  }

 private:
  SuccessType succ_val_;
};

template <class E>
class Either<void, E> : private detail::EitherConstraints<void, E> {
 public:
  using SuccessType = void;
  using ErrorType = E;

  Either() = delete;

  template <class EE = ErrorType,
            class = std::enable_if_t<std::is_copy_constructible<EE>::value>>
  explicit constexpr Either(ErrorType const& err_val) noexcept(
      std::is_nothrow_copy_constructible<ErrorType>::value)
      : err_val_(err_val) {}

  template <class EE = ErrorType,
            class = std::enable_if_t<std::is_move_constructible<EE>::value>>
  explicit constexpr Either(ErrorType&& err_val) noexcept(
      std::is_nothrow_move_constructible<ErrorType>::value)
      : err_val_(std::move(err_val)) {}

  constexpr auto IsSuccess() const noexcept -> bool { return false; }
  constexpr auto IsError() const noexcept -> bool { return true; }

  constexpr operator bool() const noexcept { return false; }

  constexpr auto Success() const -> SuccessType {
    throw BadEitherAccess("[et::Either<void, E>::Success]");
  }

  //  constexpr auto Error() & noexcept -> ErrorType& { return err_val_; }
  constexpr auto Error() const& noexcept -> ErrorType const& {
    return err_val_;
  }

  constexpr auto Error() && noexcept -> ErrorType&& {
    return std::move(err_val_);
  }
  constexpr auto Error() const&& noexcept -> ErrorType const&& {
    return std::move(err_val_);
  }

 protected:
  ErrorType err_val_;
};

template <class SS>
constexpr auto Success(SS&& succ_val) {
  return Either<std::decay_t<SS>, void>(std::forward<SS>(succ_val));
}

template <class EE>
constexpr auto Error(EE&& err_val) {
  return Either<void, std::decay_t<EE>>(std::forward<EE>(err_val));
}

template <class S, class E>
class Either final : private detail::Storage<S, E>,
                     detail::EitherConstraints<S, E> {
 private:
  using Base = detail::Storage<S, E>;

 public:
  using SuccessType = typename Base::SuccessType;
  using ErrorType = typename Base::ErrorType;

  constexpr operator bool() const noexcept { return this->IsSuccess(); }

  template <class BB = Base,
            class = std::enable_if_t<std::is_copy_assignable<BB>::value>>
  constexpr Either(Either const& that) noexcept(
      std::is_nothrow_copy_assignable<Base>::value) {
    *this = that;
  }

  template <class BB = Base,
            class = std::enable_if_t<std::is_move_assignable<BB>::value>>
  constexpr Either(Either&& that) noexcept(
      std::is_move_assignable<Base>::value) {
    *this = std::move(that);
  }

  template <class BB = Base,
            class = std::enable_if_t<std::is_copy_assignable<BB>::value>>
  constexpr Either& operator=(Either const& that) noexcept(
      std::is_nothrow_copy_assignable<Base>::value) {
    static_cast<Base>(*this) = static_cast<Base const&>(that);
    return *this;
  }

  template <class BB = Base,
            class = std::enable_if_t<std::is_move_assignable<BB>::value>>
  constexpr Either& operator=(Either&& that) {
    static_cast<Base>(*this) = static_cast<Base&&>(that);
    return *this;
  }

  // conversion constructors
  template <class SS = SuccessType,
            class = std::enable_if_t<std::is_copy_constructible<SS>::value>>
  constexpr Either(Either<SuccessType, void> const& that) noexcept(
      std::is_nothrow_copy_constructible<SuccessType>::value)
      : Base(detail::SuccessTag, that.Success()) {}

  template <class SS = SuccessType,
            class = std::enable_if_t<std::is_move_constructible<SS>::value>>
  constexpr Either(Either<SuccessType, void>&& that) noexcept(
      std::is_nothrow_move_constructible<SuccessType>::value)
      : Base(detail::SuccessTag,
             static_cast<Either<SuccessType, void>>(that).Success()) {}

  template <class EE = ErrorType,
            class = std::enable_if_t<std::is_copy_constructible<EE>::value>>
  constexpr Either(Either<void, ErrorType> const& that) noexcept(
      std::is_nothrow_copy_constructible<ErrorType>::value)
      : Base(detail::ErrorTag, that.Error()) {}

  template <class EE = ErrorType,
            class = std::enable_if_t<std::is_move_constructible<EE>::value>>
  constexpr Either(Either<void, ErrorType>&& that) noexcept(
      std::is_nothrow_move_constructible<ErrorType>::value)
      : Base(detail::ErrorTag,
             static_cast<Either<void, ErrorType>>(that).Error()) {}

  // conversion assignment
  template <class SS = SuccessType,
            class = std::enable_if_t<std::is_copy_assignable<SS>::value>>
  constexpr Either& operator=(Either<SuccessType, void> const& that) noexcept(
      detail::meta::Conjuction<
          std::is_nothrow_destructible<Base>::value,
          std::is_nothrow_copy_assignable<SuccessType>::value>::value) {
    this->state_ = detail::StorageState::kHasSuccess;
    this->succ_val_ = that.Success();

    return *this;
  }

  template <class SS = SuccessType,
            class = std::enable_if_t<std::is_move_assignable<SS>::value>>
  constexpr Either& operator=(Either<SuccessType, void>&& that) noexcept(
      detail::meta::Conjuction<
          std::is_nothrow_destructible<Base>::value,
          std::is_nothrow_move_assignable<SuccessType>::value>::value) {
    this->state_ = detail::StorageState::kHasError;
    this->succ_val_ =
        std::move(static_cast<Either<SuccessType, void>&&>(that).Success());

    return *this;
  }

  template <class EE = ErrorType,
            std::enable_if_t<std::is_copy_assignable<EE>::value>>
  constexpr Either& operator=(Either<void, ErrorType> const& that) noexcept(
      detail::meta::Conjuction<
          std::is_nothrow_destructible<Base>::value,
          std::is_nothrow_copy_assignable<ErrorType>::value>::value) {
    this->state_ = detail::StorageState::kHasError;
    this->err_val_ = that.Error();

    return *this;
  }

  template <class EE = ErrorType,
            std::enable_if_t<std::is_move_assignable<EE>::value>>
  constexpr Either& operator=(Either<void, ErrorType>&& that) noexcept(
      detail::meta::Conjuction<
          std::is_nothrow_destructible<Base>::value,
          std::is_nothrow_move_assignable<ErrorType>::value>::value) {
    this->steate_ = detail::StorageState::kHasError;
    this->err_val_ =
        std::move(static_cast<Either<void, ErrorType>&&>(that).Error());

    return *this;
  }

  // Access
  constexpr auto IsSuccess() const noexcept -> bool {
    return this->state_ == detail::StorageState::kHasSuccess;
  }
  constexpr auto IsError() const noexcept -> bool {
    return this->state_ == detail::StorageState::kHasError;
  }

  //  constexpr auto Success() & -> SuccessType& {
  //    return this->state_ == detail::StorageState::kHasSuccess
  //               ? this->succ_val_
  //               : (throw BadEitherAccess(
  //                     "[et::Either<S, E>::Success] invalid state access"));
  //  }

  constexpr auto Success() const& -> SuccessType const& {
    return this->state_ == detail::StorageState::kHasSuccess
               ? this->succ_val_
               : (throw BadEitherAccess(
                     "[et::Either<S, E>::Success] invalid state access"));
  }

  constexpr auto Success() && -> SuccessType&& {
    if (this->state_ == detail::StorageState::kHasSuccess) {
      this->state_ = detail::StorageState::kEmpty;
      return std::move(this->succ_val_);
    } else {
      throw BadEitherAccess("[et::Either<S, E>::Success] invalid state access");
    }
  }

  constexpr auto Success() const&& -> SuccessType const&& {
    if (this->state_ == detail::StorageState::kHasSuccess) {
      this->state_ = detail::StorageState::kEmpty;
      return std::move(this->succ_val_);
    } else {
      throw BadEitherAccess("[et::Either<S, E>::Success] invalid state access");
    }
  }

  //  constexpr auto Error() & -> ErrorType& {
  //    return this->state_ == detail::StorageState::kHasError
  //               ? this->err_val_
  //               : (throw BadEitherAccess(
  //                     "[et::Either<S, E>::Error] invalid state access"));
  //  }

  constexpr auto Error() const& -> ErrorType const& {
    return this->state_ == detail::StorageState::kHasError
               ? this->err_val_
               : (throw BadEitherAccess(
                     "[et::Either<S, E>::Error] invalid state access"));
  }

  constexpr auto Error() && -> ErrorType&& {
    if (this->state_ == detail::StorageState::kHasError) {
      this->state_ = detail::StorageState::kEmpty;
      return std::move(this->err_val_);
    } else {
      throw BadEitherAccess("[et::Either<S, E>::Error] invalid state access");
    }
  }

  constexpr auto Error() const&& -> ErrorType const&& {
    if (this->state_ == detail::StorageState::kHasError) {
      this->state_ = detail::StorageState::kEmpty;
      return std::move(this->err_val_);
    } else {
      throw BadEitherAccess("[et::Either<S, E>::Error] invalid state access");
    }
  }

  template <class Fn, class SS, class EE,
            class = std::enable_if_t<
                std::is_invocable_v<Fn, Either<SuccessType, ErrorType>>>>
  [[nodiscard]] constexpr auto Map(Fn&& f) noexcept(
      std::is_nothrow_invocable_v<Fn, Either<SuccessType, ErrorType>>) {
    return Map(*this);
  }
};

template <class S, class E>
bool operator==(Either<S, E> const& lhs, Either<S, E> const& rhs) noexcept;

template <class S>
bool operator==(Either<S, void> const& lhs,
                Either<S, void> const& rhs) noexcept {
  return lhs.Success() == rhs.Success();
}

template <class E>
bool operator==(Either<void, E> const& lhs,
                Either<void, E> const& rhs) noexcept {
  return rhs.Error() == rhs.Error();
}

template <class S, class E>
bool operator==(Either<S, void> const& lhs,
                Either<void, E> const& rhs) noexcept {
  return false;
}

template <class S, class E>
bool operator==(Either<S, E> const& lhs, Either<S, E> const& rhs) noexcept {
  if (lhs.IsSuccess() && rhs.IsSuccess()) {
    return lhs.Success() == rhs.Success();
  } else if (lhs.IsError() == rhs.IsError()) {
    return lhs.Error() == rhs.Error();
  }

  return false;
}

template <class S, class E>
bool operator!=(Either<S, E> const& lhs, Either<S, E> const& rhs) noexcept {
  return !(lhs == rhs);
}

template <class S, class E>
std::ostream& operator<<(std::ostream&, Either<S, E> const&);

template <class S>
std::ostream& operator<<(std::ostream& os, Either<S, void> const& e) {
  return os << e.Success();
}

template <class E>
std::ostream& operator<<(std::ostream& os, Either<void, E> const e) {
  return os << e.Error();
}

template <class S, class E>
std::ostream& operator<<(std::ostream& os, Either<S, E> const& e) {
  if (e) {
    return os << e.Success();
  } else {
    return os << e.Error();
  }
}
}  // namespace et

#endif  // ET_EITHER_HPP_
