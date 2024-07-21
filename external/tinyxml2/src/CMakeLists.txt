
add_library(tinyxml2-static STATIC tinyxml2.cpp)

target_include_directories(tinyxml2-static PUBLIC ${CMAKE_SOURCE_DIR}/header_only_include)
target_include_directories(tinyxml2-static PUBLIC ../include)

target_link_libraries(tinyxml2-static PRIVATE re2::re2)
target_link_libraries(tinyxml2-static PRIVATE spdlog::spdlog)
