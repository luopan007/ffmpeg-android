#!/bin/bash
export NDK_HOME=${ANDROID_NDK}
export PLATFORM_VERSION=android-16
function build
{
	echo "start build ffmpeg for $ARCH"
	./configure --target-os=linux \
	--prefix=$PREFIX --arch=$ARCH \
	--disable-doc \
	--enable-shared \
	--disable-static \
	--disable-yasm \
	--disable-asm \
	--disable-symver \
	--enable-gpl \
	--disable-ffmpeg \
	--disable-ffplay \
	--disable-ffprobe \
	--disable-ffserver \
	--cross-prefix=$CROSS_COMPILE \
	--enable-cross-compile \
	--sysroot=$SYSROOT \
	--enable-small \
	--extra-cflags="-Os -fpic $ADDI_CFLAGS" \
	--extra-ldflags="$ADDI_LDFLAGS" \
	$ADDITIONAL_CONFIGURE_FLAG
	make clean
	make
	make install
	echo "build ffmpeg for $ARCH finished"
}

#arm
ARCH=arm
CPU=arm
PREFIX=/Users/luopan/Documents/ffmpeg-github/ffmpeg-2.8.15/android/arm
TOOLCHAIN=$NDK_HOME/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64
CROSS_COMPILE=$TOOLCHAIN/bin/arm-linux-androideabi-
ADDI_CFLAGS="-marm"
SYSROOT=$NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH/
build

#x86
ARCH=x86
CPU=x86
PREFIX=/Users/luopan/Documents/ffmpeg-github/ffmpeg-2.8.15/android/arm/x86
TOOLCHAIN=$NDK_HOME/toolchains/x86-4.9/prebuilt/darwin-x86_64
CROSS_COMPILE=$TOOLCHAIN/bin/i686-linux-android-
ADDI_CFLAGS="-march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32"
SYSROOT=$NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH/
build
