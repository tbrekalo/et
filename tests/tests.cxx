#include <cstddef>
#include <string>

#include "catch2/catch_test_macros.hpp"
#include "et/either.hpp"

TEST_CASE("Either constexpr Success construction",
          "[either][constructor][constexpr][Success]") {
  auto constexpr et_val = 12;
  auto constexpr et = et::Either<decltype(et_val), void>(et_val);

  CHECK(et);
  CHECK(et.IsSuccess());
  CHECK_FALSE(et.IsError());
  CHECK(et.Success() == et_val);
}

TEST_CASE("Either constexpr Error construction",
          "[either][constructor][constexpr][Error]") {
  auto constexpr et_val = 'c';
  auto constexpr et = et::Either<void, decltype(et_val)>(et_val);

  CHECK_FALSE(et);
  CHECK_FALSE(et.IsSuccess());
  CHECK(et.IsError());
  CHECK(et.Error() == et_val);
}

TEST_CASE("Either constexpr Success factory function",
          "[either][factory][constexpr][Success]") {
  auto constexpr et_val = 12;
  auto constexpr et = et::Success(et_val);

  static_assert(
      std::is_same<std::remove_cv_t<decltype(et)>,
                   et::Either<std::remove_cv_t<decltype(et_val)>, void>>::value,
      "");

  CHECK(et.Success() == et_val);
}

TEST_CASE("Either constexpr Error factory function",
          "[either][factory][constexpr][Error]") {
  auto constexpr et_val = 'c';
  auto constexpr et = et::Error(et_val);

  static_assert(
      std::is_same<std::remove_cv_t<decltype(et)>,
                   et::Either<void, std::remove_cv_t<decltype(et_val)>>>::value,
      "");

  CHECK(et.Error() == et_val);
}

TEST_CASE("Either constexpr Success up cast constructor",
          "[either][factory][constexpr][upcast][Success]") {
  auto constexpr et_val = 12;
  auto constexpr et =
      et::Either<std::remove_cv_t<decltype(et_val)>, char>(et::Success(et_val));

  CHECK(et);
  CHECK(et.IsSuccess());
  CHECK_FALSE(et.IsError());
  CHECK(et.Success() == et_val);
  CHECK_THROWS_AS(et.Error(), et::BadEitherAccess);
}

TEST_CASE("Either constexpr Error up cast assignment",
          "[either][factory][constexpr][upcast][Error]") {
  auto constexpr et_val = 'c';
  auto constexpr et =
      et::Either<std::int32_t, std::remove_cv_t<decltype(et_val)>>(
          et::Error(et_val));

  CHECK_FALSE(et);
  CHECK_FALSE(et.IsSuccess());
  CHECK(et.IsError());
  CHECK(et.Error() == et_val);
  CHECK_THROWS_AS(et.Success(), et::BadEitherAccess);
}

TEST_CASE("Either constexpr Success upcast assign",
          "[either][factory][constexpr][upcast][Error][assignment]") {
  auto constexpr et_val = 12;
  et::Either<std::remove_cv_t<decltype(et_val)>, char> et = et::Success(et_val);

  CHECK(et);
  CHECK(et.IsSuccess());
  CHECK_FALSE(et.IsError());
  CHECK(et.Success() == et_val);
  CHECK_THROWS_AS(et.Error(), et::BadEitherAccess);
}

TEST_CASE("Either constexpr Error up cast assign",
          "[either][factory][constexpr][upcast][Error]") {
  auto constexpr et_val = 'c';
  et::Either<std::int32_t, std::remove_cv_t<decltype(et_val)>> constexpr et =
      et::Error(et_val);

  CHECK_FALSE(et);
  CHECK_FALSE(et.IsSuccess());
  CHECK(et.IsError());
  CHECK(et.Error() == et_val);
  CHECK_THROWS_AS(et.Success(), et::BadEitherAccess);
}

TEST_CASE("Either assignment copy 1", "[either][constexpr][assignment]") {
  auto constexpr et_val = 12;
  auto constexpr et1 =
      et::Either<std::remove_cv_t<decltype(et_val)>, char>(et::Success(et_val));

  auto constexpr et2 = et1;

  CHECK(et2);
  CHECK(et2.IsSuccess());
  CHECK_FALSE(et2.IsError());
  CHECK(et2.Success() == et_val);
  CHECK_THROWS_AS(et2.Error(), et::BadEitherAccess);
}

TEST_CASE("Either assignment copy 2", "[either][constexpr][assignment]") {
  auto constexpr et_val = 'c';
  auto constexpr et1 =
      et::Either<std::int32_t, std::remove_cv_t<decltype(et_val)>>(
          et::Error(et_val));

  auto constexpr et2 = et1;

  CHECK_FALSE(et2);
  CHECK_FALSE(et2.IsSuccess());
  CHECK(et2.IsError());
  CHECK(et2.Error() == et_val);
  CHECK_THROWS_AS(et2.Success(), et::BadEitherAccess);
}

TEST_CASE("Either assignment move 1", "[either][constexpr][assignment]") {
  auto const et_val = std::string("HelloHelloHelloHelloHelloHelloHello");
  auto et1 =
      et::Either<std::remove_cv_t<decltype(et_val)>, char>(et::Success(et_val));

  auto const et2 = std::move(et1);

  CHECK(et2);
  CHECK(et2.IsSuccess());
  CHECK_FALSE(et2.IsError());
  CHECK(et2.Success() == et_val);
  CHECK_THROWS_AS(et2.Error(), et::BadEitherAccess);

  CHECK_FALSE(et1.IsSuccess());
  CHECK_FALSE(et1.IsError());
}

TEST_CASE("Either assignment move 2", "[either][constexpr][assignment]") {
  auto et_val = std::string("HelloHelloHelloHelloHelloHelloHello");
  auto et1 = et::Either<std::int32_t, std::remove_cv_t<decltype(et_val)>>(
      et::Error(et_val));

  auto et2 = std::move(et1);

  CHECK_FALSE(et2);
  CHECK_FALSE(et2.IsSuccess());
  CHECK(et2.IsError());
  CHECK(et2.Error() == et_val);
  CHECK_THROWS_AS(et2.Success(), et::BadEitherAccess);

  CHECK_FALSE(et1.IsSuccess());
  CHECK_FALSE(et1.IsError());
}
