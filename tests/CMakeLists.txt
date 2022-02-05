add_executable(checksum_test checksum_test.cpp)
target_link_libraries(checksum_test ${PROJECT_NAME} gtest)

add_executable(file_test file_test.cpp)
target_link_libraries(file_test ${PROJECT_NAME} gtest)
