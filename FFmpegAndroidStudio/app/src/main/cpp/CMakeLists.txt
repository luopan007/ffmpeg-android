# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# 定义头文件的路径
include_directories(
        ${CMAKE_SOURCE_DIR}/include/
        ${CMAKE_SOURCE_DIR}/
)

add_library(
        # 库名字
        native-lib
        # 共享库
        SHARED
        # 库文件原始文件
        ${CMAKE_SOURCE_DIR}/native-lib.cpp
        ${CMAKE_SOURCE_DIR}/IDemux.cpp
        ${CMAKE_SOURCE_DIR}/FFDemux.cpp
        ${CMAKE_SOURCE_DIR}/XData.cpp
        ${CMAKE_SOURCE_DIR}/XLog.cpp
        ${CMAKE_SOURCE_DIR}/XThread.cpp
        ${CMAKE_SOURCE_DIR}/IObserver.cpp
        ${CMAKE_SOURCE_DIR}/IDecode.cpp
        ${CMAKE_SOURCE_DIR}/FFDecode.cpp
        ${CMAKE_SOURCE_DIR}/XParameter.cpp
        ${CMAKE_SOURCE_DIR}/IVideoView.cpp
        ${CMAKE_SOURCE_DIR}/GLVideoView.cpp
        ${CMAKE_SOURCE_DIR}/XTexture.cpp
        ${CMAKE_SOURCE_DIR}/XEGL.cpp
        ${CMAKE_SOURCE_DIR}/XShader.cpp
        ${CMAKE_SOURCE_DIR}/IResample.cpp
        ${CMAKE_SOURCE_DIR}/FFResample.cpp
        ${CMAKE_SOURCE_DIR}/IAudioPlay.cpp
        ${CMAKE_SOURCE_DIR}/SLAudioPlay.cpp
        ${CMAKE_SOURCE_DIR}/IPlayer.cpp
)

add_library(avcodec SHARED IMPORTED)
add_library(avdevice SHARED IMPORTED)
add_library(avfilter SHARED IMPORTED)
add_library(avformat SHARED IMPORTED)
add_library(avutil SHARED IMPORTED)
add_library(swresample SHARED IMPORTED)
add_library(swscale SHARED IMPORTED)
add_library(postproc SHARED IMPORTED)

set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libavcodec.so)
set_target_properties(avdevice PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libavdevice.so)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libavfilter.so)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libavformat.so)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libavutil.so)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libswresample.so)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libswscale.so)
set_target_properties(postproc PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libpostproc.so)

find_library(
        log-lib
        log)

# 把所有的库都链接到native-lib中去
target_link_libraries( # Specifies the target library.
        native-lib
        android  #android的库需要编译进来
        OpenSLES #android的音频播放框架依赖库
        GLESv2 #android的OpenGL ES version 2
        EGL #OpenGL ES与native window的适配器
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        swresample
        swscale
        postproc
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})