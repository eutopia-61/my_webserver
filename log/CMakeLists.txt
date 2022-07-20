set(cur_dir ${CMAKE_CURRENT_SOURCE_DIR})

#include_directories(
#    include
#)

# 获取指定目录下源文件列表，保存到 DIR_SRCS 变量中，后续编译构建目标可执行文件
aux_source_directory(${cur_dir}/. DIR_SRCS)

add_library(log ${DIR_SRCS})    # 默认是静态库STATIC