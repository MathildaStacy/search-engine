add_library(Cache_manager-static STATIC keywordLRUCache.cc keywordCacheManager.cc 
                                        urlLRUCache.cc     urlCacheManager.cc)

target_include_directories(Cache_manager-static PUBLIC ${CMAKE_SOURCE_DIR}/header_only_include)
target_include_directories(Cache_manager-static PUBLIC ../include)

target_link_libraries(Cache_manager-static PRIVATE re2::re2)
target_link_libraries(Cache_manager-static PRIVATE spdlog::spdlog)
target_link_libraries(Cache_manager-static PRIVATE tinyxml2-static)
target_link_libraries(Cache_manager-static PUBLIC dictProducer-static)
target_link_libraries(Cache_manager-static PUBLIC PageLibPreprocessor-static)
target_link_libraries(Cache_manager-static PUBLIC reactor-static)