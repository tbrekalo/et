# constexpr Either <S, E>

**C++14** header only result monad implementation.

## Features

- constexpr support
- 0 dependencies
- single header

## Status

- in development
- TODOs:
    - expand test coverage
    - stricter compile time constraints
    - support for function return types
    - support std::hash for either

## Example

```c++
#include <et/either.hpp>
#include <iostream>
#include <string>
#include <vector>

auto Example(std::int32_t val) -> et::Either<std::int32_t, std::string> {
  using namespace std::string_literals;
  if (val > 0) {
    return et::Success(2 * val);
  } else {
    return et::Error("[example::Example] error: val must be > 0"s);
  }
}

int main(void) {
  std::cout << Example(2) << std::endl;
  std::cout << Example(-1) << std::endl;

  std::vector<et::Either<std::int32_t, char>> vec = {
      et::Success(312), et::Error('v'), et::Success(420)};

  return EXIT_SUCCESS;
}
```