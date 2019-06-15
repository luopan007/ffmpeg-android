# ffmpeg-android
ffmpeg for android

1、ffmpeg-2.8.5目录下的build_android.sh是编译android平台so的脚本
2、build_android.sh脚本中的export NDK_HOME是指定Android NDK根目录，需要自行从Android官网下载。
3、export PLATFORM_VERSION是指定android的版本，如果编译不通过，可以修改为你主机上存在的版本。
4、PREFIX是指定编译后的安装目录
