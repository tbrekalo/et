#include <cstddef>
#include <string>

#include "catch2/catch_test_macros.hpp"
#include "et/either.hpp"

TEST_CASE("Either constructor literals", "[either][constructor][literal]") {
  constexpr auto e1 = et::Success(12);
  constexpr et::Either<std::int32_t, char> et2 = et::Success(12);
}