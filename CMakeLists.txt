cmake_minimum_required(VERSION 3.17)
project(et VERSION 0.1.0 LANGUAGES CXX
  DESCRIPTION "C++ implementation of compound algebraic data type inspired by Scala's Either<S, E>"
)

include(cmake/Warnings.cmake)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_14)
set_default_warnings(${PROJECT_NAME} INTERFACE FALSE)

if (ASan)
  target_compile_options(${PROJECT_NAME} INTERFACE -fsanitize=address -fno-omit-frame-pointer)
  target_link_options(${PROJECT_NAME} INTERFACE -fsanitize=address)
endif()

if (BUILD_TESTS) 
  add_subdirectory(tests) 
endif()
