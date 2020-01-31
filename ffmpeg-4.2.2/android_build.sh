#!/bin/bash
set -x
# 目标Android版本
API=21
CPU=armv7-a
#so库输出目录
OUTPUT=/Users/luopan/Desktop/ffmpeg-4.2.2/android/$CPU
# NDK的路径，根据自己的NDK位置进行设置
NDK=/Users/luopan/opt/android-ndk-r20b
# 编译工具链路径
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/darwin-x86_64
# 编译环境
SYSROOT=$TOOLCHAIN/sysroot

function build
{
  ./configure \
  --prefix=$OUTPUT \
  --target-os=android \
  --arch=arm \
  --cpu=$CPU \
  --enable-asm \
  --enable-neon \
  --enable-small \
  --enable-gpl \
  --enable-runtime-cpudetect \
  --enable-jni \
  --enable-mediacodec \
  --enable-decoder=h264_mediacodec \
  --enable-hwaccel=h264_mediacodec \
  --enable-cross-compile \
  --enable-shared \
  --disable-static \
  --disable-doc \
  --disable-ffplay \
  --disable-ffprobe \
  --disable-symver \
  --disable-ffmpeg \
  --sysroot=$SYSROOT \
  --cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
  --cross-prefix-clang=$TOOLCHAIN/bin/armv7a-linux-androideabi$API- \
  --extra-cflags="-fPIC"

  make clean all
  # 这里是定义用几个CPU编译
  make -j12
  make install
}

build
