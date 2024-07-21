

add_library(dictProducer-static STATIC DictProducer.cc)

target_include_directories(dictProducer-static PUBLIC ${CMAKE_SOURCE_DIR}/header_only_include)
target_include_directories(dictProducer-static PUBLIC ../include)

target_link_libraries(dictProducer-static PRIVATE re2::re2)
target_link_libraries(dictProducer-static PRIVATE spdlog::spdlog)
target_link_libraries(dictProducer-static PUBLIC add-static)