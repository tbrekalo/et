#ifndef ET_EITHER_HPP_
#define ET_EITHER_HPP_

#include <cstdint>
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

}  // namespace detail

constexpr detail::SuccessTagImpl SuccessTag;
constexpr detail::ErrorTagImpl ErrorTag;

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
  constexpr Storage(SuccessTagType, SuccessType const& succ_val) noexcept(
      std::is_nothrow_copy_constructible<SuccessType>::value)
      : state_(StorageState::kHasSuccess), succ_val_(succ_val) {}

  constexpr Storage(SuccessTagType, SuccessType&& succ_val) noexcept(
      std::is_nothrow_move_constructible<SuccessType>::value)
      : state_(StorageState::kHasSuccess), succ_val_(std::move(succ_val)) {}

  constexpr Storage(ErrorTagType, ErrorType const& err_val) noexcept(
      std::is_nothrow_copy_constructible<ErrorType>::value)
      : state_(StorageState::kHasError), err_val_(err_val) {}

  constexpr Storage(ErrorTagType, ErrorType&& err_val) noexcept(
      std::is_nothrow_move_constructible<ErrorType>::value)
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
  constexpr Storage(SuccessTagType, SuccessType const& succ_val) noexcept(
      std::is_nothrow_copy_constructible<SuccessType>::value)
      : state_(StorageState::kHasSuccess), succ_val_(succ_val) {}

  constexpr Storage(SuccessTagType, SuccessType&& succ_val) noexcept(
      std::is_nothrow_move_constructible<SuccessType>::value)
      : state_(StorageState::kHasSuccess), succ_val_(std::move(succ_val)) {}

  constexpr Storage(ErrorTagType, ErrorType const& err_val) noexcept(
      std::is_nothrow_copy_constructible<ErrorType>::value)
      : state_(StorageState::kHasError), err_val_(err_val) {}

  constexpr Storage(ErrorTagType, ErrorType&& err_val) noexcept(
      std::is_nothrow_move_constructible<ErrorType>::value)
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

  static_assert(not std::is_reference<SuccessType>::value,
                "[et::Either] Either<SuccessType&, ErrorType> ill formed");

  static_assert(not std::is_reference<ErrorType>::value,
                "[et::Either] Either<SuccessType, ErrorType&> ill formed");
};

}  // namespace detail

template <class S>
class Either<S, void> final : detail::EitherConstraints<S, void> {
 public:
  using SuccessType = S;
  using ErrorType = void;

  explicit constexpr Either(SuccessType const& succ_val) noexcept(
      std::is_nothrow_copy_constructible<SuccessType>::value)
      : succ_val_(succ_val) {}

  explicit constexpr Either(SuccessType&& succ_val) noexcept(
      std::is_nothrow_move_constructible<ErrorType>::value)
      : succ_val_(std::move(succ_val)) {}

  constexpr auto IsSuccess() const noexcept -> bool { return true; }
  constexpr auto IsError() const noexcept -> bool { return false; }

  constexpr auto Success() & noexcept -> SuccessType& { return succ_val_; }
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
class Either<void, E> {
 public:
  using SuccessType = void;
  using ErrorType = E;

 private:
  static constexpr auto kNoThrowCopyCtor =
      std::is_nothrow_copy_constructible<ErrorType>::value;
  static constexpr auto kNoThrowMoveCtor =
      std::is_nothrow_move_constructible<ErrorType>::value;

 public:
  explicit constexpr Either(ErrorType const& err_val_) noexcept(
      kNoThrowCopyCtor)
      : err_val_(err_val_) {}

  explicit constexpr Either(ErrorType&& err_val) noexcept(kNoThrowMoveCtor)
      : err_val_(std::move(err_val)) {}

  constexpr auto IsSuccess() const noexcept -> bool { return false; }
  constexpr auto IsError() const noexcept -> bool { return true; }

  constexpr auto Success() const -> SuccessType {
    throw BadEitherAccess("[et::Either<void, E>::Success]");
  }

  constexpr auto Error() & noexcept -> ErrorType& { return err_val_; }
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
class Either final : private detail::Storage<S, E> {
 private:
  using Base = detail::Storage<S, E>;

 public:
  using SuccessType = typename Base::SuccessType;
  using ErrorType = typename Base::ErrorType;

  constexpr operator bool() const noexcept { return this->IsSuccess(); }

  constexpr Either(Either const&) = default;
  constexpr Either(Either&&) = default;

  constexpr Either& operator=(Either const&) = default;
  constexpr Either& operator=(Either&&) = default;

  constexpr Either(Either<SuccessType, void> const& that) noexcept(
      std::is_nothrow_copy_constructible<SuccessType>::value)
      : Base(SuccessTag, that.Success()) {}

  constexpr Either(Either<SuccessType, void>&& that) noexcept(
      std::is_nothrow_move_constructible<SuccessType>::value)
      : Base(SuccessTag,
             static_cast<Either<SuccessType, void>>(that).Success()) {}

  constexpr Either(Either<void, ErrorType> const& that) noexcept(
      std::is_nothrow_copy_constructible<ErrorType>::value)
      : Base(ErrorTag, that.Error()) {}

  constexpr Either(Either<void, ErrorType>&& that) noexcept(
      std::is_nothrow_move_constructible<ErrorType>::value)
      : Base(ErrorTag, static_cast<Either<void, ErrorType>>(that).Error()) {}

  // Access
  constexpr auto IsSuccess() const noexcept -> bool {
    return this->state_ == detail::StorageState::kHasSuccess;
  }
  constexpr auto IsError() const noexcept -> bool {
    return this->state_ == detail::StorageState::kHasError;
  }

  constexpr auto Success() & -> SuccessType& {
    return this->is_success_
               ? this->succ_val_
               : (throw BadEitherAccess(
                     "[et::Either<S, E>::Success] invalid state access"));
  }

  constexpr auto Success() const& -> SuccessType const& {
    return this->is_success_
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

  constexpr auto Error() & -> ErrorType& {
    return this->state_ == detail::StorageState::kHasError
               ? this->err_val_
               : (throw BadEitherAccess(
                     "[et::Either<S, E>::Error] invalid state access"));
  }

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
};

}  // namespace et

#endif  // ET_EITHER_HPP_
