

add_executable(app main.cc)

add_executable(client client.cc)

target_include_directories(app PUBLIC ../header_only_include)
target_include_directories(client PUBLIC ../header_only_include)

target_link_libraries(client PRIVATE re2::re2)
target_link_libraries(client PRIVATE spdlog::spdlog)

target_link_libraries(app PRIVATE re2::re2)
target_link_libraries(app PRIVATE spdlog::spdlog)
target_link_libraries(app PUBLIC add-static)
target_link_libraries(app PUBLIC dictProducer-static)
target_link_libraries(app PUBLIC PageLibPreprocessor-static)
target_link_libraries(app PUBLIC reactor-static)
target_link_libraries(app PUBLIC Cache_manager-static)
