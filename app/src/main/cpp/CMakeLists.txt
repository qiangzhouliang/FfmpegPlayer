# 指定最低版本
cmake_minimum_required(VERSION 3.22.1)

project("ffmpegplayer")

# 指定引入头文件
include_directories(${CMAKE_SOURCE_DIR}/../jniLibs/include)

# 指定共享库目录
link_directories(${CMAKE_SOURCE_DIR}/../jniLibs/${CMAKE_ANDROID_ARCH_ABI})

# src/main/cpp 目录下所有的.cpp 文件都需要编译进来
#aux_source_directory(${CMAKE_SOURCE_DIR}/*.cpp, SOURCE)
file(GLOB SOURCE ${CMAKE_SOURCE_DIR}/*.cpp)

# 生成 ffmpegplayer
add_library(
        ffmpegplayer
        SHARED

        ${SOURCE})

# 设置一个环境变量 目录为 armeabi-v7a
#set(my_lib_path ${CMAKE_SOURCE_DIR}/../jniLibs/${CMAKE_ANDROID_ARCH_ABI})
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${my_lib_path}")
# 解决 java.lang.UnsatisfiedLinkError: dlopen failed
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -frtti -fopenmp -static-openmp")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -frtti -fopenmp -static-openmp")

find_library(log-lib log)

# 为 ffmpegplayer 添加FFmpeg 编译链接库
target_link_libraries(
        ffmpegplayer
        -Wl,--start-group # 忽略顺序引发的问题
        avcodec-57 avdevice-57 avfilter-6 avformat-57 avutil-55 postproc-54 swresample-2 swscale-4  # 指明要编译那些静态库
        -Wl,--end-group
        ${log-lib}
        android
        z
        OpenSLES)