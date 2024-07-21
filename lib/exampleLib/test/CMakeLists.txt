

add_executable(exampleTest exampleTest.cc)
target_link_libraries(exampleTest add-static gtest gmock)

target_include_directories(exampleTest PRIVATE ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
target_include_directories(exampleTest PRIVATE ${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})