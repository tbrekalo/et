find_package(Catch2 3 QUIET)
if (NOT Catch2_FOUND)
  include(FetchContent)

  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.0.0-preview3
  )

  FetchContent_MakeAvailable(Catch2)
endif()

set(${PROJECT_NAME}_TESTS_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/tests.cxx
)

add_executable(${PROJECT_NAME}_TESTS ${${PROJECT_NAME}_TESTS_SOURCES})
target_link_libraries(${PROJECT_NAME}_TESTS 
  PRIVATE
    ${PROJECT_NAME}
    Catch2::Catch2WithMain
)
