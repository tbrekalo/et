#ifndef ET_EITHER_HPP_
#define ET_EITHER_HPP_

#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace et {

// forward declaration for friendship
template <class S, class E>
class Either;

// forward declarations for friendship
template <class SS>
constexpr Either<SS, void> Success(SS);

template <class EE>
constexpr Either<void, EE> Err(EE);

namespace detail {

struct SuccessTagImpl {};
struct ErrTagImpl {};

using SuccessTagType = SuccessTagImpl const&;
using ErrTagType = ErrTagImpl const&;

}  // namespace detail

constexpr detail::SuccessTagImpl SuccessTag;
constexpr detail::ErrTagImpl ErrTag;

class BadEitherAccess : public std::logic_error {
 public:
  explicit BadEitherAccess(char const* msg) : std::logic_error(msg) {}
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

#define STEAL_SUCCESS(ST) std::move(ST).Success()
#define STEAL_ERROR(ST) std::move(ST).Err()

template <class S, class E>
class UnitStorage;

// NOTE: should never be instantiated
// only to avoid ambiguous partial specialization error
template <>
class UnitStorage<void, void> {};

template <class S, class E>
class UnionStorage {
 public:
  using SuccessType = S;
  using ErrorType = E;

  static_assert(meta::All<meta::NotVoid, SuccessType, ErrorType>::value,
                "[et::detail::UnionStorage] void type");

 protected:
  template <class T>
  static constexpr auto kNoThrowCopyCtor =
      std::is_nothrow_copy_constructible<T>::value;

  template <class T>
  static constexpr auto kNoThrowMoveCtor =
      std::is_nothrow_move_constructible<T>::value;

 public:
  // TODO: constructors, only copy and move
  UnionStorage(UnitStorage<SuccessType, void>&& that)
      : is_success_(true), succ_val_(STEAL_SUCCESS(that)) {}

  UnionStorage(UnitStorage<void, ErrorType>&& that)
      : is_success_(false), err_val_(STEAL_ERROR(that)) {}

  constexpr auto IsSuccess() const noexcept -> bool { return is_success_; }
  constexpr auto IsError() const noexcept -> bool { return !is_success_; }

  constexpr auto Success() & -> SuccessType& {
    return is_success_
               ? succ_val_
               : (throw BadEitherAccess(
                     "[et::Either<S, E>::Success] invalid state access"));
  }

  constexpr auto Success() const& -> SuccessType const& {
    return is_success_
               ? succ_val_
               : (throw BadEitherAccess(
                     "[et::Either<S, E>::Success] invalid state access"));
  }

  constexpr auto Success() && -> SuccessType&& {
    return is_success_
               ? std::move(succ_val_)
               : (throw BadEitherAccess(
                     "[et::Either<S, E>::Success] invalid state access"));
  }

  constexpr auto Err() & -> ErrorType& {
    return !is_success_ ? err_val_
                        : (throw BadEitherAccess(
                              "[et::Either<S, E>::Err] invalid state access"));
  }

  constexpr auto Err() const& -> ErrorType const& {
    return !is_success_ ? err_val_
                        : (throw BadEitherAccess(
                              "[et::Either<S, E>::Err] invalid state access"));
  }

  constexpr auto Err() && -> ErrorType&& {
    return !is_success_ ? std::move(err_val_)
                        : (throw BadEitherAccess(
                              "[et::Either<S, E>::Err] invalid state access"));
  }

 protected:
  bool is_success_;
  union {
    SuccessType succ_val_;
    ErrorType err_val_;
  };
};

template <class S>
class UnitStorage<S, void> {
 public:
  using SuccessType = S;
  using ErrorType = void;

  static_assert(meta::NotVoid<SuccessType>::value,
                "[et::detail::UnitStorage<S, void>] S is void");

  UnitStorage(SuccessType const& succ_val) : succ_val_(succ_val) {}
  UnitStorage(SuccessType&& succ_val) : succ_val_(std::move(succ_val)) {}

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

  constexpr auto Err() const -> ErrorType {
    throw BadEitherAccess("[et::Either<S, void>::Err]");
  }

 protected:
  SuccessType succ_val_;
};

template <class E>
class UnitStorage<void, E> {
 public:
  using SuccessType = void;
  using ErrorType = E;

  static_assert(meta::NotVoid<E>::value,
                "[et::detail::UnitStorage<void, E>] E is void");

  UnitStorage(ErrorType const& err_val_) : err_val_(err_val_) {}
  UnitStorage(ErrorType&& err_val) : err_val_(std::move(err_val)) {}

  constexpr auto IsSuccess() const noexcept -> bool { return false; }
  constexpr auto IsError() const noexcept -> bool { return true; }

  constexpr auto Success() const -> SuccessType {
    throw BadEitherAccess("[et::Either<void, E>::Success]");
  }

  constexpr auto Err() & noexcept -> ErrorType& { return err_val_; }
  constexpr auto Err() const& noexcept -> ErrorType const& { return err_val_; }

  constexpr auto Err() && noexcept -> ErrorType&& {
    return std::move(err_val_);
  }
  constexpr auto Err() const&& noexcept -> ErrorType const&& {
    return std::move(err_val_);
  }

 protected:
  ErrorType err_val_;
};

template <class S, class E>
using PickStorage = std::conditional_t<meta::All<meta::NotVoid, S, E>::value,
                                       UnionStorage<S, E>, UnitStorage<S, E>>;

}  // namespace detail

template <class S, class E>
class Either final : public detail::PickStorage<S, E> {
 private:
  using Base = detail::PickStorage<S, E>;

  template <class... Args>
  Either(Args&&... args) : Base(std::forward<Args>(args)...) {}

 public:
  using SuccessType = typename Base::SuccessType;
  using ErrorType = typename Base::ErrorType;

  Either(detail::UnitStorage<SuccessType, void>&& that) : Base(that) {}
  Either(detail::UnitStorage<void, ErrorType>&& that) : Base(that) {}

  constexpr operator bool() const noexcept { return this->IsSuccess(); }

 private:
  // constraints
  static_assert(
      not detail::meta::Conjuction<std::is_void<SuccessType>::value,
                                   std::is_void<ErrorType>::value>::value,
      "[et::Either] Either<void, void> ill formed");

  static_assert(not std::is_reference<SuccessType>::value,
                "[et::Either] Either<SuccessType&, ErrorType> ill formed");

  static_assert(not std::is_reference<ErrorType>::value,
                "[et::Either] Either<SuccessType, ErrorType&> ill formed");
};

template <class SS>
constexpr Either<SS, void> Success(SS succ_val) {
  static_assert(!std::is_same<detail::UnitStorage<void, void>,
                              detail::UnitStorage<SS, void>>::value,
                "[er::Success]");

  return Either<SS, void>(
      detail::UnitStorage<SS, void>(std::forward<SS>(std::move(succ_val))));
}

template <class EE>
constexpr Either<void, EE> Err(EE err_val) {
  static_assert(!std::is_same<detail::UnitStorage<void, void>,
                              detail::UnitStorage<void, EE>>::value,
                "[et::Err]");

  return Either<void, EE>(detail::UnitStorage<void, EE>(std::move(err_val)));
}

}  // namespace et

#endif  // ET_EITHER_HPP_
