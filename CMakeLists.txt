cmake_minimum_required(VERSION 3.22.1)

# 设置编译器为gcc/g++
SET(CMAKE_C_COMPILER /usr/bin/gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/g++)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(cpp_test)
# 设置必须跟上面的一样，即c++17

# 设置生成 compile_commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 显示编译时间
set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time")

# 设置成debug模式
set(CMAKE_BUILD_TYPE "Debug")
# 设置调试的指令
add_compile_options(-Wall -Wextra -Wshadow -Wpedantic) # fsanitize=undefined -fsanitize=address
#add_compile_options(-Werror)

add_subdirectory(external/googletest)
add_subdirectory(external/googlebenchmark)
add_subdirectory(external/re2)
add_subdirectory(external/spdlog)
add_subdirectory(external/tinyxml2)
add_subdirectory(lib)
add_subdirectory(app)

add_subdirectory(test)

# 查找clang-format命令
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    # 定义要格式化的文件
    file(GLOB_RECURSE ALL_SOURCE_FILES 
        *.cpp
        *.h
        *.cc
        *.hpp)

    # 添加一个自定义目标来运行clang-format
    add_custom_target(
        clang-format
        COMMAND ${CLANG_FORMAT}
        -i
        -style=file
        ${ALL_SOURCE_FILES}
        COMMENT "Running clang-format on all source files"
    )
else()
    message(STATUS "clang-format not found!")
endif()
