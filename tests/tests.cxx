#include "catch2/catch_test_macros.hpp"
#include "et/either.hpp"

#include <cstddef>
#include <string>

TEST_CASE("Either constructor literals", "[either][constructor][literal]") {
  std::int32_t i = et::Success(12).Success();
//  et::Either<std::int32_t, char> e1(et::Success(12));
//  et::Either<std::int32_t, char> e2(et::Err(' '));
//
//  CHECK(e1.IsSuccess());
//  CHECK_FALSE(e2.IsError());
//  CHECK(e1);
//
//  CHECK_FALSE(e2.IsSuccess());
//  CHECK(e2.IsError());
//  CHECK_FALSE(e2);
}