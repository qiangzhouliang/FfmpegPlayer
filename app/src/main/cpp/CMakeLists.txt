cmake_minimum_required(VERSION 3.22.1)

project("ffmpegplayer")

# 引入头文件
include_directories(${CMAKE_SOURCE_DIR}/include)
# src/main/cpp 目录下所有的.cpp 文件都需要编译进来
file(GLOB SOURCE ${CMAKE_SOURCE_DIR}/*.cpp)

add_library(
        ffmpegplayer
        SHARED

        ${SOURCE})

# 设置一个环境变量 目录为 armeabi-v7a
set(my_lib_path ${CMAKE_SOURCE_DIR}/../../../libs/${CMAKE_ANDROID_ARCH_ABI})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${my_lib_path}")

find_library(
        log-lib

        log)

target_link_libraries(
        ffmpegplayer
        -Wl,--start-group # 忽略顺序引发的问题
        avcodec avfilter avformat avutil swresample swscale  # 指明要编译那些静态库
        -Wl,--end-group
        ${log-lib}
        android
        z
        OpenSLES)