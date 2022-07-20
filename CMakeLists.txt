cmake_minimum_required(VERSION 3.8)

set(This my_webserer)
project(${This} C CXX)

set(cur_dir ${CMAKE_CURRENT_SOURCE_DIR})

# SET(CMAKE_BUILD_TYPE "Debug")

# 设置语言标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lpthread -O0 -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O0 -pthread -Wall -g")

# 设置版本信息
set(META_PROJECT_NAME ${This})
set(META_VERSION_MAJOR "1")
set(META_VERSION_MINOR "0")
set(META_VERSION_PATCH "0")
set(META_VERSION "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}")
set(META_NAME_VERSION "${META_PROJECT_NAME} v${META_VERSION}")

# Create version file
file(WRITE "${cur_dir}/VERSION" "${META_NAME_VERSION}")


# 设置输出目录
set(OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/output")
set(EXECUTABLE_OUTPUT_PATH "${OUTPUT_PATH}/bin/")
set(LIBRARY_OUTPUT_PATH "${OUTPUT_PATH}/lib/") 

# 分别设置Debug和Release输出目录
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Bin/Debug)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Bin/Release)

# Add header file include directories
include_directories(
    ${cur_dir}/pool
    ${cur_dir}/timer
    ${cur_dir}/server
    ${cur_dir}/buffer
    ${cur_dir}/http
    ${cur_dir}/log
)

# Add block directories
add_subdirectory(buffer)
add_subdirectory(http)
add_subdirectory(timer)
add_subdirectory(server)
add_subdirectory(log)


# 生成可执行文件
add_executable(server main.cpp)

# 链接库文件
target_link_libraries(server webserver httpconn timer buffer log)