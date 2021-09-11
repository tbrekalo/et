add_executable(${PROJECT_NAME}_example ${CMAKE_CURRENT_SOURCE_DIR}/example.cxx)
target_link_libraries(${PROJECT_NAME}_example PRIVATE ${PROJECT_NAME})