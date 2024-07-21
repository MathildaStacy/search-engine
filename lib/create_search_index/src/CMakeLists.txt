add_library(PageLibPreprocessor-static STATIC PageLibPreprocessor.cc)

target_include_directories(PageLibPreprocessor-static PUBLIC ${CMAKE_SOURCE_DIR}/header_only_include)
target_include_directories(PageLibPreprocessor-static PUBLIC ../include)

target_link_libraries(PageLibPreprocessor-static PRIVATE re2::re2)
target_link_libraries(PageLibPreprocessor-static PRIVATE spdlog::spdlog)
target_link_libraries(PageLibPreprocessor-static PRIVATE tinyxml2-static)
target_link_libraries(PageLibPreprocessor-static PUBLIC add-static)