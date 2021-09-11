cmake_minimum_required(VERSION 3.17)
project(et VERSION 0.1.0 LANGUAGES CXX
        DESCRIPTION "C++14 header only result monad implementation."
        )

include(cmake/Warnings.cmake)

add_library(${PROJECT_NAME} INTERFACE examples/example.cxx)
target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_14)
set_default_warnings(${PROJECT_NAME} INTERFACE FALSE)

if (ASan)
    target_compile_options(${PROJECT_NAME} INTERFACE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(${PROJECT_NAME} INTERFACE -fsanitize=address)
endif ()

if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()
